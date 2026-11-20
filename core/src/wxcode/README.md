# WX Code Format and Decoding

## 1. Data Layout

- The grid is `G[72][16]`, with angular index `a=0..71` and radial index `r=0..15`.
- Cells contain 0 or 1; 1 represents dark ink. Other values are rejected.
- Angles increase clockwise from the left; radial indices increase outward.
- Traverse angular rows first, then radial cells. Read bits most significant
  first and assemble each group of 8 bits into a codeword.
- The payload is a length-delimited byte sequence and may contain NUL bytes.

Format and data positions use `FORMAT_WORDS`, `FORMAT_MASK`, and `DATA_MASKS` in
[WXFormatInformation.cpp](WXFormatInformation.cpp), preserving their values and
array order.

```text
Image -> Detection and sampling -> Format decoding -> Extraction and unmasking
      -> Deinterleaving -> RS correction -> Segment parsing -> Payload conversion
```

## 2. Format Information

Format cells occupy `r=6` at odd angular indices, giving 36 bits. Read them as
two consecutive 18-bit values, `F0` and `F1`, without applying the data mask.

For each of the 512 format codewords, compute:

```text
d(i) = min(popcount(F0 XOR FORMAT_WORDS[i]),
           popcount(F1 XOR FORMAT_WORDS[i]))
```

Select the index with the smallest distance, choosing the smallest index on a
tie. Reject if the minimum distance exceeds 3. Extract the fields as follows:

```text
v      = (i >> 7) & 3     // Version 0–3
ecBits = (i >> 5) & 3
m      = i & 31           // Mask 0–31
e      = ecBits XOR 1     // Correction profile: 0=L, 1=M, 2=Q, 3=H
```

## 3. Data Extraction and Masks

If bit `r` of `DATA_MASKS[72*v+a]` is set, read `G[a][r]` and XOR it with
`Mask(m,a,r)`. Assemble codewords in grid traversal order. Other cells do not
enter the data bitstream.

Let `s=a+r`, `p=a*r`, and `h=a/3+r/2`. Division is integer division; comparisons
evaluate to 0 or 1. The operators `^`, `&`, `|`, and `~` are bitwise.

```text
 m   Mask(m,a,r)
 0   ((s & 1) ^ (a >= 10 && a <= 50)) ^ 1
 1   (s % 3 == 0) ^ (a < 51)
 2   ((s & 1) ^ ((a & 3) != 0)) ^ 1
 3   ((s & 1) ^ (a % 3 != 0)) ^ 1
 4   ((a & 1) ^ (s % 3 == 0)) ^ 1
 5   ((s & 1) ^ (r > 4)) ^ 1
 6   (p % 6 == 0) ^ (a % 3 != 0)
 7   ((a & 0x1ca) == 0) ^ (r & 1)
 8   ((p / 3) & 1) ^ (a % 3 == 0)
 9   (s % 3 == 0) ^ (a % 3 != 0)
10   (r & 1) ^ (a < 51)
11   a < 51
12   (s % 3 == 0) ^ ((a & 3) != 0)
13   (~s) & 1
14   (p % 6 == 0) ^ (r & 1)
15   (~h) & 1
16   (((a / 3) & 1) ^ (s % 3 == 0)) ^ 1
17   (~((a / 3) ^ r)) & 1
18   (h ^ s) & 1
19   ((p / 3) & 1) ^ (r > 4)
20   ((h & 1) ^ ((a / 3) % 3 == 0)) ^ 1
21   (s & 1) ^ ((a & 1) | (a < 10) | (r < 5))
22   (s % 3 == 0) ^ ((a / 3) % 3 == 0)
23   (h & 1) ^ (a % 3 != 0)
24   (s & 1) ^ (a < 10)
25   ((s / 3) & 1) ^ (a % 3 == 0)
26   (h & 1) ^ (a < 10 || a > 50)
27   (s % 3 == 0) ^ (p % 6 == 0)
28   ((s + p + p / 3) & 1) ^ ((a & 3) == 0)
29   (~(p / 3)) & 1
30   (h & 1) ^ ((a & 3) == 0)
31   ((s + p + p / 3) ^ h) & 1
```

## 4. Codewords and Error Correction

Each profile is `(B,K,P)`: the number of blocks, data codewords per block, and
correction codewords per block. The total is `B*(K+P)`, including correction
bytes and segment headers; it is not the net payload capacity.

```text
Version  Total       L           M           Q           H
   0      38     (1,28,10)  (1,25,13)  (1,20,18)  (1,14,24)
   1      52     (1,38,14)  (1,35,17)  (2,13,13)  (2,10,16)
   2      66     (1,46,20)  (1,43,23)  (2,17,16)  (2,12,21)
   3      52     (1,38,14)  (1,35,17)  (2,13,13)  (2,10,16)
```

Deinterleave the unmasked codeword sequence `W` as follows:

```text
block[b][j] = W[j*B+b]
0 <= b < B; 0 <= j < K+P
```

The first `K` bytes of each block are data; the last `P` bytes are correction
codewords. Use GF(256), primitive polynomial `0x11d`, and primitive element
`alpha=2`, with coefficients ordered from highest to lowest degree:

```text
g(x) = product(x - alpha^j), j=0..P-1
```

Each block corrects up to `floor(P/2)` erroneous codewords. All syndromes must
be zero after correction; otherwise reject. Concatenate the first `K` bytes of
each corrected block in block order to obtain the segment bitstream.

## 5. Segment Parsing

The mode occupies 4 bits; segments need not be byte-aligned. All four versions
use the same count-field widths.

```text
Mode  Meaning            Following fields
 0    Terminator         Stop parsing
 1    Numeric            10-bit character count
 2    Alphanumeric        9-bit character count
 3    Structured append  Consume 16 bits; do not retain them
 4    Byte                8-bit byte count
 5    FNC1               Enable FNC1 processing
 7    ECI                See below
 8    Kanji               8-bit double-byte character count
 9    FNC1               Enable FNC1; consume no application indicator byte
13    Hanzi               4-bit subset, then 8-bit double-byte character count
```

Reject unknown modes. Stop when fewer than 4 bits remain. Padding after mode 0
is not checked.

### 5.1 Numeric and Alphanumeric

Numeric groups of three characters occupy 10 bits; trailing groups of two or
one occupy 7 or 4 bits. Values must be below 1000, 100, or 10, respectively.
Preserve leading zeros in the output.

Alphanumeric mode uses this 45-character alphabet:

```text
0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:
```

A pair occupies 11 bits with value `45*x+y`; a trailing single character
occupies 6 bits. Reject out-of-range values. With FNC1 enabled, replace `%%`
with `%` and other `%` characters with `0x1d` within the current segment, then
apply the conversion in Section 6.

### 5.2 Kanji and Hanzi

Read a 13-bit value `z` per character and emit the high byte followed by the
low byte:

```text
Kanji: t=(z/192)*256+z%192
       t += (t < 0x1f00 ? 0x8140 : 0xc140)

Hanzi: t=(z/96)*256+z%96
       t += (t < 0x03bf ? 0xa1a1 : 0xa6a1)
```

Hanzi supports subset 1 only. For other subsets, the current implementation
consumes only the subset and count fields, then resumes mode parsing without
skipping the character content.

### 5.3 ECI

- First byte below 128: use it directly as the assignment number.
- First byte 128–191: use its low 6 bits as the high bits, then read 8 bits.
- First byte 192–223: use its low 5 bits as the high bits, then read 16 bits.
- First byte 224–255: reject.

Valid assignments are 0–30, excluding 14 and 19, plus 170. Validate the field
without transcoding or retaining ECI metadata.

### 5.4 Truncation and Capacity

Reject reads with insufficient bits, except for this byte-mode behavior:
clamp the count to `ceil(remaining_bits/8)`. If initially fewer than 8 bits
remain, emit those bits as one byte value. Otherwise, continue reading 8 bits
per byte and fail if a later read has insufficient bits.

Accumulated output and converted payloads are limited to 160 bytes. Empty
payloads are not returned as scan results. Structured append does not assemble
messages across frames.

## 6. Alphanumeric Payload Conversion

After each alphanumeric segment and FNC1 processing, convert **all output
accumulated so far**. If any byte is outside the source alphabet, leave the
output unchanged.

The source is the 45-character alphabet in Section 5.1. The target alphabet is:

```text
0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#$&'()*+,/:;=?@-._~
```

Conversion steps:

1. Count `L` leading source characters `0`. Map each remaining character to its
   source-alphabet index `d`.
2. Set `value=0`. For each digit, compute `value=value*45+d`. While `value>255`,
   append its low 8 bits to an intermediate sequence and shift it right by 8.
   After consuming the input, append `value` if nonzero.
3. Set `value=0`. Consume intermediate bytes `b` in reverse order and compute
   `value=value*256+b`. While `value>=82`, append target character `value%82`
   and divide `value` by 82 using integer division. After consuming all bytes,
   append the corresponding target character if `value` is nonzero.
4. Reverse the output from step 3, prepend `L` ASCII `0` characters, and replace
   the accumulated output.

## 7. Image Sampling

Use an Otsu threshold to locate three circular finder patterns and fit a
geometric mapping. Canonical finder centers are `(-q,-q)`, `(q,-q)`, and
`(-q,q)`, where `q=31.819807/2`.

For cell `(a,r)`, take 25 samples with `da,dr=0..4`:

```text
theta = (a + (da-2)/5.0)*pi/36
rho   = 10+r+(dr+0.5)/5
(x,y) = (-cos(theta)*rho, -sin(theta)*rho)
G[a][r] = (dark_sample_count > 25*(0.53301+((4-r)/11.0)*0.3))
```

After mapping, use the nearest pixel. Discard candidates with out-of-bounds
samples. The four corners correspond to canonical points `(-27,-27)`,
`(27,-27)`, `(27,27)`, and `(-27,27)`, including the outer margin.

Each detection pass is limited to 16384 pixels per side, 33554432 pixels in
total, 128 finder candidates, and 128 sampled triples. Images that exceed the
size or finder limits are skipped. At the sampling limit, candidates already
sampled are kept for decoding.

## 8. Usage

Build with `-DZXING_ENABLE_WXCODE=ON`. The option is off by default and requires
`ZXING_READERS=ON`. Enabled builds include WX Code in default scanning; it can
also be selected explicitly:

```cpp
#include "ZXing/ZXingCpp.h"

auto image = ZXing::ImageView(pixels, width, height, ZXing::ImageFormat::Lum);
auto options = ZXing::ReaderOptions().formats(ZXing::BarcodeFormat::WXCode);
auto results = ZXing::ReadBarcodes(image, options);
```

`bytes()` returns the payload; `position()` returns the four corners in input
image coordinates. The C API uses `ZXing_BarcodeFormat_WXCode`. String filters
accept `WXCode`, `WX_CODE`, `WX Code`, `WeChatCode`, and `WeChat Code`.

Downscaling and inversion are supported. For WX Code, `Binarizer::LocalAverage`
(the default, including when explicitly selected) uses Otsu thresholding. Other
binarizer values use the shared binary bitmap. The reader does not participate in
morphological closing. `symbol()` and the AIM identifier are empty. AppID,
pages, and mini-program names are not resolved.


## 9. Reference Provenance and Maintenance

The format tables and compatibility rules were recovered from the ARM64
`libwechatQrMod.so` supplied with the original project. The reference locations
are retained here so that code comments can focus on decoding constraints:

- Format words: ELF virtual address `0x1b0004`; data masks: `0x1af704`.
  These `.data` addresses map to file offsets by subtracting `0x2000`.
- Version/error-correction profiles: constructor `0x132430`; format decoder:
  `0x133858`; format-cell mask: read-only virtual address/file offset `0x37998`.
- Data-mask formulas: function `0x135af8`. The independent mask digests were
  captured through the original function on 2026-11-20, in angular-major,
  radial-minor order, using FNV-1a over 1152 byte values per mask.
- Streaming payload conversion: helpers `0x171a24` and `0x171d14`.
  The payload golden digest uses helper `0x172050` with 1000 identical input
  sequences; the original outputs were also compared byte-for-byte on device.

Preserve table order, stream bit order, threshold values, and the compatibility
rules in Sections 5 and 6 when reorganizing the implementation. Test encoders
and golden values remain independent references, rather than using the
production correction or payload-conversion helpers to compute expectations.

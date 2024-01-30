# zxing-cpp

zxing-cpp is a Rust wrapper for the C++ library [zxing-cpp](https://github.com/zxing-cpp/zxing-cpp).

It is an open-source, multi-format linear/matrix barcode image processing library implemented in C++.
It was originally ported from the Java ZXing Library but has been developed further and now includes
many improvements in terms of runtime and detection performance.

## Usage

In your Cargo.toml:

```toml
[dependencies]
# `bundled` causes cargo to compile and statically link an up to
# date version of the c++ core library. This is the most convenient
# and safe way to build the library.
zxing-cpp = { version = "0.2.2", features = ["bundled", "image"] }
```

Simple example usage:

```rust
use zxing_cpp::{ImageView, ReaderOptions, BarcodeFormat, read_barcodes};

fn main() -> anyhow::Result<()> {
	let image = image::open("some-image-file.jpg")?;
	let opts = ReaderOptions::default()
		.formats(BarcodeFormat::QRCode | BarcodeFormat::LinearCodes)
		.try_invert(false);

	let barcodes = read_barcodes(&image, &opts)?;

	if barcodes.is_empty() {
		println!("No barcode found.");
	} else {
		for barcode in barcodes {
			println!("{}: {}", barcode.format(), barcode.text());
		}
	}

	Ok(())
}
```

Note: This should currently be considered a pre-release. The API may change slightly to be even more
"rusty" depending on community feedback.

## Optional Features

zxing-cpp provides features that are behind [Cargo features](https://doc.rust-lang.org/cargo/reference/manifest.html#the-features-section).
They are:

* `bundled` uses a bundled version of the [zxing-cpp](https://github.com/zxing-cpp/zxing-cpp) c++ library.
* [`image`](https://crates.io/crates/image) allows convenient `ImageView` construction from `GreyImage` and `DynamicImage`.

## Benchmarking

To compare the performance of this Rust wrapper project with other availble barcode scanner Rust libraries,
I started the project [zxing-bench](https://github.com/axxel/zxing-bench). The README contains a few
results to get an idea.

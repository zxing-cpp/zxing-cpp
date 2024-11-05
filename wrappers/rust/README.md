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
zxing-cpp = { version = "0.4.1", features = ["bundled", "image"] }
```

Simple example reading some barcodes from a jpg file:

```rust
use zxingcpp::BarcodeFormat;

fn main() -> anyhow::Result<()> {
	let image = image::open("some-image-file.jpg")?;

	let read_barcodes = zxingcpp::read()
		.formats(BarcodeFormat::QRCode | BarcodeFormat::LinearCodes)
		.try_invert(false);

	let barcodes = read_barcodes.from(&image)?;

	for barcode in barcodes {
		println!("{}: {}", barcode.format(), barcode.text());
	}

	Ok(())
}
```

Simple example creating a barcode and writing it to a svg file:

```rust
fn main() -> anyhow::Result<()> {
	let svg = zxingcpp::create(zxingcpp::BarcodeFormat::QRCode)
		.from_str("https://github.com/zxing-cpp/zxing-cpp")?
		.to_svg_with(&zxingcpp::write().scale(5))?;
	std::fs::write("zxingcpp.svg", svg)?;
	Ok(())
}
```

Note: This should currently be considered a pre-release. The API may change slightly to be even more
idiomatic rust depending on community feedback.

## Optional Features

zxing-cpp provides features that are behind [Cargo features](https://doc.rust-lang.org/cargo/reference/manifest.html#the-features-section).
They are:

* `bundled` uses a bundled version of the [zxing-cpp](https://github.com/zxing-cpp/zxing-cpp) c++ library.
* [`image`](https://crates.io/crates/image) allows convenient/implicit conversion between `ImageView`/`Image` and`GreyImage`/`DynamicImage`.

## Benchmarking

To compare the performance of this Rust wrapper project with other available barcode scanner Rust libraries,
I started the project [zxing-bench](https://github.com/axxel/zxing-bench). The README contains a few
results to get an idea.

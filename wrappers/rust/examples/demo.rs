/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

use clap::Parser;
use std::path::PathBuf;
use zxing_cpp::*;

#[derive(Parser)]
struct Cli {
	filename: PathBuf,
	formats: Option<String>,
	fast: bool,
}

fn main() -> anyhow::Result<()> {
	let cli = Cli::parse();

	let image = image::open(&cli.filename)?;

	#[cfg(feature = "image")]
	let iv = ImageView::try_from(&image)?;
	#[cfg(not(feature = "image"))]
	let lum_img = image.into_luma8();
	#[cfg(not(feature = "image"))]
	let iv = ImageView::from_slice(&lum_img, lum_img.width(), lum_img.height(), ImageFormat::Lum)?;

	let formats = barcode_formats_from_string(cli.formats.unwrap_or_default())?;
	let opts = ReaderOptions::new()
		.formats(formats)
		.try_harder(!cli.fast)
		.try_invert(!cli.fast)
		.try_rotate(!cli.fast);

	let results = read_barcodes(&iv, &opts)?;

	if results.is_empty() {
		println!("No barcode found.");
	} else {
		for result in results {
			println!("Text: {}", result.text());
			println!("Bytes: {:?}", result.bytes());
			println!("Format: {}", result.format());
			println!("Content: {}", result.content_type());
			println!("Identifier: {}", result.symbology_identifier());
			println!("EC Level: {}", result.ec_level());
			println!("Error: {}", result.error_message());
			println!("Orientation: {}", result.orientation());
			println!("Position: {}", result.position());
			println!();
		}
	}

	Ok(())
}

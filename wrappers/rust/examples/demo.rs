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

	#[cfg(not(feature = "image"))]
	let lum_img = image.into_luma8();
	#[cfg(not(feature = "image"))]
	let iv = ImageView::from_slice(&lum_img, lum_img.width(), lum_img.height(), ImageFormat::Lum)?;

	let formats = barcode_formats_from_string(cli.formats.unwrap_or_default())?;
	let opts = ReaderOptions::default()
		.formats(formats)
		.try_harder(!cli.fast)
		.try_invert(!cli.fast)
		.try_rotate(!cli.fast);

	#[cfg(feature = "image")]
	let barcodes = read_barcodes(&image, &opts)?;
	#[cfg(not(feature = "image"))]
	let barcodes = read_barcodes(iv, opts)?;

	if barcodes.is_empty() {
		println!("No barcode found.");
	} else {
		for barcode in barcodes {
			println!("Text:       {}", barcode.text());
			println!("Bytes:      {:?}", barcode.bytes());
			println!("Format:     {}", barcode.format());
			println!("Content:    {}", barcode.content_type());
			println!("Identifier: {}", barcode.symbology_identifier());
			println!("EC Level:   {}", barcode.ec_level());
			println!("Error:      {}", barcode.error_message());
			println!("Rotation:   {}", barcode.orientation());
			println!("Position:   {}", barcode.position());
			println!();
		}
	}

	Ok(())
}

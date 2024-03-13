/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

use zxingcpp::*;

fn main() -> anyhow::Result<()> {
	let filename = std::env::args().nth(1).expect("no image file name provided");
	let formats = std::env::args().nth(2);
	let fast = std::env::args().nth(3).is_some();

	let image = image::open(&filename)?;

	#[cfg(not(feature = "image"))]
	let lum_img = image.into_luma8();
	#[cfg(not(feature = "image"))]
	let iv = ImageView::from_slice(&lum_img, lum_img.width(), lum_img.height(), ImageFormat::Lum)?;

	let formats = BarcodeFormats::from_str(formats.unwrap_or_default())?;
	let read_barcodes = BarcodeReader::new()
		.formats(formats)
		.try_harder(!fast)
		.try_invert(!fast)
		.try_rotate(!fast)
		.try_downscale(!fast)
		.return_errors(true);

	#[cfg(feature = "image")]
	let barcodes = read_barcodes.from(&image)?;
	#[cfg(not(feature = "image"))]
	let barcodes = read_barcodes.from(iv)?;

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
			println!("Error:      {}", barcode.error());
			println!("Rotation:   {}", barcode.orientation());
			println!("Position:   {}", barcode.position());
			println!();
		}
	}

	Ok(())
}

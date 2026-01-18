/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

use image;
use std::fs;
use zxingcpp::*;

fn main() -> anyhow::Result<()> {
	let text = std::env::args().nth(1).expect("no input text provided");
	let format_str = std::env::args().nth(2).expect("no format provided");
	let filename = std::env::args().nth(3).expect("no output file name provided");

	let format = match BarcodeFormat::from_str(format_str) {
		Ok(f) => f,
		Err(e) => {
			eprintln!("Error: {}", e);
			eprintln!("Supported formats: {}", BarcodeFormats::list(BarcodeFormat::AllCreatable));
			return Err(anyhow::anyhow!("invalid barcode format"));
		}
	};
	println!("Creating barcode of format {:?} for text '{}'", format, text);
	let create_barcode = create(format);
	let bc = create_barcode.from_str(text)?;

	if filename.ends_with(".svg") {
		fs::write(filename, bc.to_svg_with(&write().add_hrt(true))?).expect("Unable to write file");
	} else {
		image::GrayImage::from(&bc.to_image_with(&write().scale(4))?).save(filename)?;
	}

	Ok(())
}

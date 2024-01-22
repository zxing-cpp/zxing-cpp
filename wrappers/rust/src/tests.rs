/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#[cfg(test)]
mod tests {
	use crate::*;

	#[test]
	fn barcode_formats_from_string_valid() {
		let formats = barcode_formats_from_string("qrcode,linearcodes").unwrap();
		assert_eq!(formats, BarcodeFormat::QRCode | BarcodeFormat::LinearCodes);
	}

	#[test]
	fn reader_options() {
		let mut o1 = ReaderOptions::default();
		assert_eq!(o1.get_formats(), BarcodeFormat::None);
		assert_eq!(o1.get_try_harder(), true);
		o1.set_formats(BarcodeFormat::EAN8);
		assert_eq!(o1.get_formats(), BarcodeFormat::EAN8);
		o1.set_try_harder(false);
		assert_eq!(o1.get_try_harder(), false);

		o1 = ReaderOptions::default().is_pure(true).text_mode(TextMode::Hex);
		assert_eq!(o1.get_formats(), BarcodeFormat::None);
		assert_eq!(o1.get_try_harder(), true);
		assert_eq!(o1.get_is_pure(), true);
		assert_eq!(o1.get_text_mode(), TextMode::Hex);
	}

	#[test]
	#[should_panic]
	fn barcode_formats_from_string_invalid() {
		let _ = barcode_formats_from_string("qrcoder").unwrap();
	}

	#[test]
	fn read_barcodes_pure() {
		let mut data = Vec::<u8>::new();
		for v in "0000101000101101011110111101011011101010100111011100101000100101110010100000".chars() {
			data.push(if v == '0' { 255 } else { 0 });
		}
		let iv = ImageView::from_slice(&data, data.len(), 1, ImageFormat::Lum).unwrap();
		let op = ReaderOptions::default().binarizer(Binarizer::BoolCast);
		let res = read_barcodes(&iv, &op).unwrap();

		let expected = "96385074";

		assert_eq!(res.len(), 1);
		assert_eq!(res[0].is_valid(), true);
		assert_eq!(res[0].format(), BarcodeFormat::EAN8);
		assert_eq!(res[0].text(), expected);
		assert_eq!(res[0].bytes(), expected.as_bytes());
		assert_eq!(res[0].has_eci(), false);
		assert_eq!(res[0].content_type(), ContentType::Text);
		assert_eq!(res[0].orientation(), 0);
		assert_eq!(res[0].position().top_left, PointI { x: 4, y: 0 });
		assert_eq!(res[0].line_count(), 1);
	}
}

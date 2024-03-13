/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#[cfg(test)]
mod tests {
	use crate::*;

	#[test]
	fn barcode_formats_from_str_valid() {
		let formats = BarcodeFormats::from_str("qrcode,linearcodes").unwrap();
		assert_eq!(formats, BarcodeFormat::QRCode | BarcodeFormat::LinearCodes);
	}

	#[test]
	fn barcode_reader_new() {
		let mut o1 = BarcodeReader::new();
		assert_eq!(o1.get_formats(), BarcodeFormat::None);
		assert_eq!(o1.get_try_harder(), true);
		o1.set_formats(BarcodeFormat::EAN8);
		assert_eq!(o1.get_formats(), BarcodeFormat::EAN8);
		o1.set_try_harder(false);
		assert_eq!(o1.get_try_harder(), false);

		o1 = BarcodeReader::new().is_pure(true).text_mode(TextMode::Hex);
		assert_eq!(o1.get_formats(), BarcodeFormat::None);
		assert_eq!(o1.get_try_harder(), true);
		assert_eq!(o1.get_is_pure(), true);
		assert_eq!(o1.get_text_mode(), TextMode::Hex);
	}

	#[test]
	fn barcode_creator_new() {
		let mut o1 = BarcodeCreator::new(BarcodeFormat::QRCode);
		assert_eq!(o1.get_reader_init(), false);
		o1.set_reader_init(true);
		assert_eq!(o1.get_reader_init(), true);
	}

	#[test]
	#[should_panic]
	fn barcode_formats_from_str_invalid() {
		let _ = BarcodeFormats::from_str("qrcoder").unwrap();
	}

	#[test]
	fn create_from_str() {
		let str = "123456";
		let res = create(BarcodeFormat::QRCode).ec_level("Q").from_str(str).unwrap();

		assert_eq!(res.is_valid(), true);
		assert_eq!(res.format(), BarcodeFormat::QRCode);
		assert_eq!(res.text(), str);
		assert_eq!(res.bytes(), str.as_bytes());
		assert_eq!(res.has_eci(), false);
		assert_eq!(res.content_type(), ContentType::Text);
		assert!(matches!(res.error(), BarcodeError::None()));
		assert_eq!(res.error().to_string(), "");
	}

	#[test]
	fn create_from_slice() {
		let data = [1, 2, 3, 4, 5];
		let res = create(BarcodeFormat::QRCode).reader_init(true).from_slice(&data).unwrap();

		assert_eq!(res.is_valid(), true);
		assert_eq!(res.format(), BarcodeFormat::QRCode);
		assert_eq!(res.bytes(), data);
		assert_eq!(res.has_eci(), true);
		// assert_eq!(res.reader_init(), true); // TODO
		assert_eq!(res.content_type(), ContentType::Binary);
		assert!(matches!(res.error(), BarcodeError::None()));
		assert_eq!(res.error().to_string(), "");
	}

	#[test]
	fn read_pure() {
		let mut data = Vec::<u8>::new();
		for v in "0000101000101101011110111101011011101010100111011100101000100101110010100000".chars() {
			data.push(if v == '0' { 255 } else { 0 });
		}
		let iv = ImageView::from_slice(&data, data.len(), 1, ImageFormat::Lum).unwrap();
		let res = read().binarizer(Binarizer::BoolCast).from(&iv).unwrap();

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
		assert!(matches!(res[0].error(), BarcodeError::None()));
		assert_eq!(res[0].error().to_string(), "");
	}
}

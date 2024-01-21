#![allow(unused_unsafe)]
#![allow(clippy::useless_transmute)]
#![allow(clippy::redundant_closure_call)]

#[allow(dead_code)]
#[allow(non_camel_case_types)]
#[allow(non_upper_case_globals)]
mod bindings {
	include!("bindings.rs");
}

use bindings::*;

use flagset::{flags, FlagSet};
use paste::paste;
use std::ffi::{c_char, c_int, c_uint, c_void, CStr, CString};
use std::fmt::{Display, Formatter};
use std::io::ErrorKind;
use std::marker::PhantomData;
use std::mem::transmute;

pub type Error = std::io::Error;

fn c2r_str(str: *mut c_char) -> String {
	let mut res = String::new();
	if !str.is_null() {
		unsafe { res = CStr::from_ptr(str).to_string_lossy().to_string() };
		unsafe { zxing_free(str as *mut c_void) };
	}
	res
}

fn c2r_vec(buf: *mut u8, len: c_int) -> Vec<u8> {
	let mut res = Vec::<u8>::new();
	if !buf.is_null() && len > 0 {
		unsafe { res = std::slice::from_raw_parts(buf, len as usize).to_vec() };
		unsafe { zxing_free(buf as *mut c_void) };
	}
	res
}

macro_rules! last_error_or {
	($expr:expr) => {
		match unsafe { zxing_LastErrorMsg().as_mut() } {
			None => Ok($expr),
			Some(error) => Err(Error::new(ErrorKind::InvalidInput, c2r_str(error))),
		}
	};
}

macro_rules! make_zxing_enum {
    ($name:ident { $($field:ident),* }) => {
        #[repr(u32)]
        #[derive(Debug, Copy, Clone)]
        pub enum $name {
            $($field = paste! { [<zxing_ $name _ $field>] },)*
        }
    }
}

macro_rules! make_zxing_flags {
    ($name:ident { $($field:ident),* }) => {
        flags! {
            #[repr(u32)]
            pub enum $name: c_uint {
                $($field = paste! { [<zxing_ $name _ $field>] },)*
            }
        }
    }
}
#[rustfmt::skip] // workaround for broken #[rustfmt::skip::macros(make_zxing_enum)]
make_zxing_enum!(ImageFormat { Lum, RGB, RGBX });
#[rustfmt::skip]
make_zxing_enum!(ContentType { Text, Binary, Mixed, GS1, ISO15434, UnknownECI });
#[rustfmt::skip]
make_zxing_enum!(Binarizer { LocalAverage, GlobalHistogram, FixedThreshold, BoolCast });
#[rustfmt::skip]
make_zxing_enum!(TextMode { Plain, ECI, HRI, Hex, Escaped });
#[rustfmt::skip]
make_zxing_enum!(EanAddOnSymbol { Ignore, Read, Require });

#[rustfmt::skip]
make_zxing_flags!(BarcodeFormat {
	None, Aztec, Codabar, Code39, Code93, Code128, DataBar, DataBarExpanded, DataMatrix, EAN8, EAN13, ITF,
	MaxiCode, PDF417, QRCode, UPCA, UPCE, MicroQRCode, RMQRCode, DXFilmEdge, LinearCodes, MatrixCodes, Any
});

pub type BarcodeFormats = FlagSet<BarcodeFormat>;

impl Display for BarcodeFormat {
	fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
		write!(f, "{}", unsafe { c2r_str(zxing_BarcodeFormatToString(BarcodeFormats::from(*self).bits())) })
	}
}

impl Display for ContentType {
	fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
		write!(f, "{}", unsafe { c2r_str(zxing_ContentTypeToString(transmute(*self))) })
	}
}

pub struct ImageView<'a>(*mut zxing_ImageView, PhantomData<&'a u8>);

impl Drop for ImageView<'_> {
	fn drop(&mut self) {
		unsafe { zxing_ImageView_delete(self.0) }
	}
}

impl<'a> AsRef<ImageView<'a>> for ImageView<'a> {
	fn as_ref(&self) -> &ImageView<'a> {
		self
	}
}

impl<'a> ImageView<'a> {
	pub fn new(data: &'a [u8], width: u32, height: u32, format: ImageFormat, row_stride: u32, pix_stride: u32) -> Self {
		unsafe {
			ImageView(
				zxing_ImageView_new(
					data.as_ptr(),
					width as c_int,
					height as c_int,
					format as zxing_ImageFormat,
					row_stride as c_int,
					pix_stride as c_int,
				),
				PhantomData,
			)
		}
	}

	pub fn cropped(self, left: i32, top: i32, width: i32, height: i32) -> Self {
		unsafe { zxing_ImageView_crop(self.0, left, top, width, height) }
		self
	}

	pub fn rotated(self, degree: i32) -> Self {
		unsafe { zxing_ImageView_rotate(self.0, degree) }
		self
	}
}

#[cfg(feature = "image")]
use image;

#[cfg(feature = "image")]
impl<'a> From<&'a image::GrayImage> for ImageView<'a> {
	fn from(img: &'a image::GrayImage) -> Self {
		ImageView::new(img.as_ref(), img.width(), img.height(), ImageFormat::Lum, 0, 0)
	}
}

#[cfg(feature = "image")]
impl<'a> TryFrom<&'a image::DynamicImage> for ImageView<'a> {
	type Error = Error;

	fn try_from(img: &'a image::DynamicImage) -> Result<Self, Error> {
		let format = match img {
			image::DynamicImage::ImageLuma8(_) => Some(ImageFormat::Lum),
			image::DynamicImage::ImageRgb8(_) => Some(ImageFormat::RGB),
			image::DynamicImage::ImageRgba8(_) => Some(ImageFormat::RGBX),
			_ => None,
		};
		match format {
			Some(format) => Ok(ImageView::new(img.as_bytes(), img.width(), img.height(), format, 0, 0)),
			None => Err(Error::new(ErrorKind::InvalidInput, "Invalid image format (must be either luma8|rgb8|rgba8)")),
		}
	}
}

pub struct ReaderOptions(*mut zxing_ReaderOptions);

impl Drop for ReaderOptions {
	fn drop(&mut self) {
		unsafe { zxing_ReaderOptions_delete(self.0) }
	}
}

impl Default for ReaderOptions {
	fn default() -> Self {
		Self::new()
	}
}

impl AsRef<ReaderOptions> for ReaderOptions {
	fn as_ref(&self) -> &ReaderOptions {
		self
	}
}

macro_rules! property {
	($name:ident, $type:ty) => {
		pub fn $name(self, v: impl Into<$type>) -> Self {
			paste! { unsafe { [<zxing_ReaderOptions_set $name:camel>](self.0, transmute(v.into())) } };
			self
		}

		paste! {
			pub fn [<set_ $name>](&mut self, v : impl Into<$type>) -> &mut Self {
				unsafe { [<zxing_ReaderOptions_set $name:camel>](self.0, transmute(v.into())) };
				self
			}

			pub fn [<get_ $name>](&self) -> $type {
				unsafe { transmute([<zxing_ReaderOptions_get $name:camel>](self.0)) }
			}
		}
	};
}

impl ReaderOptions {
	pub fn new() -> Self {
		unsafe { ReaderOptions(zxing_ReaderOptions_new()) }
	}

	property!(try_harder, bool);
	property!(try_rotate, bool);
	property!(try_invert, bool);
	property!(try_downscale, bool);
	property!(is_pure, bool);
	property!(return_errors, bool);
	property!(formats, BarcodeFormats);
	property!(text_mode, TextMode);
	property!(binarizer, Binarizer);
	property!(ean_add_on_symbol, EanAddOnSymbol);
	property!(max_number_of_symbols, i32);
}

pub struct ReaderResult(*mut zxing_Result);

impl Drop for ReaderResult {
	fn drop(&mut self) {
		unsafe { zxing_Result_delete(self.0) }
	}
}

macro_rules! getter {
	($r_name:ident, $c_name:ident, $conv:expr, $type:ty) => {
		pub fn $r_name(&self) -> $type {
			paste! { unsafe { $conv([<zxing_Result_ $c_name>](self.0)) } }
		}
	};
}

impl ReaderResult {
	getter!(is_valid, isValid, transmute, bool);
	getter!(format, format, (|f| BarcodeFormats::new(f).unwrap().into_iter().last().unwrap()), BarcodeFormat);
	getter!(content_type, contentType, transmute, ContentType);
	getter!(text, text, c2r_str, String);
	getter!(ec_level, ecLevel, c2r_str, String);
	getter!(error_message, errorMsg, c2r_str, String);
	getter!(symbology_identifier, symbologyIdentifier, c2r_str, String);
	getter!(orientation, orientation, transmute, i32);
	getter!(is_inverted, isInverted, transmute, bool);
	getter!(is_mirrored, isMirrored, transmute, bool);

	pub fn bytes(&self) -> Vec<u8> {
		let mut len: c_int = 0;
		unsafe { c2r_vec(zxing_Result_bytes(self.0, &mut len), len) }
	}
}

pub fn barcode_formats_from_string(str: impl AsRef<str>) -> Result<BarcodeFormats, Error> {
	let cstr = CString::new(str.as_ref())?;
	let res = unsafe { BarcodeFormats::new_unchecked(zxing_BarcodeFormatsFromString(cstr.as_ptr())) };
	match res.bits() {
		u32::MAX => last_error_or!(BarcodeFormats::default()),
		0 => Ok(BarcodeFormats::full()),
		_ => Ok(res),
	}
}

pub fn read_barcodes<'a>(image: impl AsRef<ImageView<'a>>, opts: impl AsRef<ReaderOptions>) -> Result<Vec<ReaderResult>, Error> {
	unsafe {
		let results = zxing_ReadBarcodes(image.as_ref().0, opts.as_ref().0);
		if !results.is_null() {
			let size = zxing_Results_size(results);
			let mut vec = Vec::<ReaderResult>::with_capacity(size as usize);
			for i in 0..size {
				vec.push(ReaderResult(zxing_Results_move(results, i)));
			}
			zxing_Results_delete(results);
			Ok(vec)
		} else {
			last_error_or!(Vec::<ReaderResult>::default())
		}
	}
}

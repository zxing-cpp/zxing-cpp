/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#![allow(unknown_lints)] // backward compatibility
#![allow(unused_unsafe)]
#![allow(clippy::useless_transmute)]
#![allow(clippy::redundant_closure_call)]
#![allow(clippy::missing_transmute_annotations)] // introduced in 1.79

mod tests;

#[allow(dead_code)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(non_upper_case_globals)]
mod bindings {
	include!("bindings.rs");
}

use bindings::*;

use flagset::{flags, FlagSet};
use paste::paste;
use std::ffi::{c_char, c_int, c_uint, c_void, CStr, CString, NulError};
use std::fmt::{Display, Formatter};
use std::marker::PhantomData;
use std::mem::transmute;
use std::rc::Rc;
use thiserror::Error;

#[derive(Error, Debug)]
pub enum Error {
	#[error("{0}")]
	InvalidInput(String),

	#[error("NulError from CString::new")]
	NulError(#[from] NulError),
	//
	// #[error("data store disconnected")]
	// IOError(#[from] std::io::Error),
	// #[error("the data for key `{0}` is not available")]
	// Redaction(String),
	// #[error("invalid header (expected {expected:?}, found {found:?})")]
	// InvalidHeader {
	//     expected: String,
	//     found: String,
	// },
	// #[error("unknown data store error")]
	// Unknown,
}

// see https://github.com/dtolnay/thiserror/issues/62
impl From<std::convert::Infallible> for Error {
	fn from(_: std::convert::Infallible) -> Self {
		unreachable!()
	}
}

fn c2r_str(str: *mut c_char) -> String {
	let mut res = String::new();
	if !str.is_null() {
		unsafe { res = CStr::from_ptr(str).to_string_lossy().to_string() };
		unsafe { ZXing_free(str as *mut c_void) };
	}
	res
}

fn c2r_vec(buf: *mut u8, len: c_int) -> Vec<u8> {
	let mut res = Vec::<u8>::new();
	if !buf.is_null() && len > 0 {
		unsafe { res = std::slice::from_raw_parts(buf, len as usize).to_vec() };
		unsafe { ZXing_free(buf as *mut c_void) };
	}
	res
}

fn last_error() -> Error {
	match unsafe { ZXing_LastErrorMsg().as_mut() } {
		None => panic!("Internal error: ZXing_LastErrorMsg() returned NULL"),
		Some(error) => Error::InvalidInput(c2r_str(error)),
	}
}

macro_rules! last_error_or {
	($expr:expr) => {
		match unsafe { ZXing_LastErrorMsg().as_mut() } {
			None => Ok($expr),
			Some(error) => Err(Error::InvalidInput(c2r_str(error))),
		}
	};
}

macro_rules! last_error_if_null_or {
	($ptr:ident, $expr:expr) => {
		match $ptr.is_null() {
			true => Err(last_error()),
			false => Ok($expr),
		}
	};
}

macro_rules! make_zxing_class {
	($r_class:ident, $c_class:ident) => {
		paste! {
			pub struct $r_class(*mut $c_class);

			impl Drop for $r_class {
				fn drop(&mut self) {
					unsafe { [<$c_class _delete>](self.0) }
				}
			}
		}
	};
}

macro_rules! make_zxing_class_with_default {
	($r_class:ident, $c_class:ident) => {
		make_zxing_class!($r_class, $c_class);
		paste! {
			impl $r_class {
				pub fn new() -> Self {
					unsafe { $r_class([<$c_class _new>]()) }
				}
			}

			impl Default for $r_class {
				fn default() -> Self {
					Self::new()
				}
			}
		}
	};
}

macro_rules! getter {
	($class:ident, $c_name:ident, $r_name:ident, $conv:expr, $type:ty) => {
		pub fn $r_name(&self) -> $type {
			paste! { unsafe { $conv([<ZXing_ $class _ $c_name>](self.0)) } }
		}
	};
	($class:ident, $c_name:ident, $conv:expr, $type:ty) => {
		paste! { getter! { $class, $c_name, [<$c_name:snake>], $conv, $type } }
	};
}

macro_rules! property {
	($class:ident, $c_name:ident, $r_name:ident, String) => {
		pub fn $r_name(self, v: impl AsRef<str>) -> Self {
			let cstr = CString::new(v.as_ref()).unwrap();
			paste! { unsafe { [<ZXing_ $class _set $c_name>](self.0, cstr.as_ptr()) } };
			self
		}

		paste! {
			pub fn [<set_ $r_name>](&mut self, v : impl AsRef<str>) -> &mut Self {
				let cstr = CString::new(v.as_ref()).unwrap();
				unsafe { [<ZXing_ $class _set $c_name>](self.0, cstr.as_ptr()) };
				self
			}

			pub fn [<get_ $r_name>](&self) -> String {
				unsafe { c2r_str([<ZXing_ $class _get $c_name>](self.0)) }
			}
		}
	};

	($class:ident, $c_name:ident, $r_name:ident, $type:ty) => {
		pub fn $r_name(self, v: impl Into<$type>) -> Self {
			paste! { unsafe { [<ZXing_ $class _set $c_name>](self.0, transmute(v.into())) } };
			self
		}

		paste! {
			pub fn [<set_ $r_name>](&mut self, v : impl Into<$type>) -> &mut Self {
				unsafe { [<ZXing_ $class _set $c_name>](self.0, transmute(v.into())) };
				self
			}

			pub fn [<get_ $r_name>](&self) -> $type {
				unsafe { transmute([<ZXing_ $class _get $c_name>](self.0)) }
			}
		}
	};

	($class:ident, $c_name:ident, $type:ty) => {
		paste! { property! { $class, $c_name, [<$c_name:snake>], $type } }
	};
}

macro_rules! make_zxing_enum {
    ($name:ident { $($field:ident),* }) => {
        #[repr(u32)]
        #[derive(Debug, Copy, Clone, PartialEq)]
        pub enum $name {
            $($field = paste! { [<ZXing_ $name _ $field>] },)*
        }
    }
}

macro_rules! make_zxing_flags {
    ($name:ident { $($field:ident),* }) => {
        flags! {
            #[repr(u32)]
            pub enum $name: c_uint {
                $($field = paste! { [<ZXing_ $name _ $field>] },)*
            }
        }
    }
}
#[rustfmt::skip] // workaround for broken #[rustfmt::skip::macros(make_zxing_enum)]
make_zxing_enum!(ImageFormat { Lum, LumA, RGB, BGR, RGBA, ARGB, BGRA, ABGR });
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
	None, Aztec, Codabar, Code39, Code93, Code128, DataBar, DataBarExpanded, DataBarLimited, DataMatrix,
	EAN8, EAN13, ITF, MaxiCode, PDF417, QRCode, UPCA, UPCE, MicroQRCode, RMQRCode, DXFilmEdge,
	LinearCodes, MatrixCodes, Any
});

impl Display for BarcodeFormat {
	fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
		write!(f, "{}", unsafe { c2r_str(ZXing_BarcodeFormatToString(BarcodeFormats::from(*self).bits())) })
	}
}

impl Display for ContentType {
	fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
		write!(f, "{}", unsafe { c2r_str(ZXing_ContentTypeToString(transmute(*self))) })
	}
}

pub type BarcodeFormats = FlagSet<BarcodeFormat>;

pub trait FromStr: Sized {
	fn from_str(str: impl AsRef<str>) -> Result<Self, Error>;
}

impl FromStr for BarcodeFormat {
	fn from_str(str: impl AsRef<str>) -> Result<BarcodeFormat, Error> {
		let cstr = CString::new(str.as_ref())?;
		let res = unsafe { BarcodeFormats::new_unchecked(ZXing_BarcodeFormatFromString(cstr.as_ptr())) };
		match res.bits() {
			u32::MAX => last_error_or!(BarcodeFormat::None),
			_ => Ok(res.into_iter().last().unwrap()),
		}
	}
}

impl FromStr for BarcodeFormats {
	fn from_str(str: impl AsRef<str>) -> Result<BarcodeFormats, Error> {
		let cstr = CString::new(str.as_ref())?;
		let res = unsafe { BarcodeFormats::new_unchecked(ZXing_BarcodeFormatsFromString(cstr.as_ptr())) };
		match res.bits() {
			u32::MAX => last_error_or!(BarcodeFormats::default()),
			0 => Ok(BarcodeFormats::full()),
			_ => Ok(res),
		}
	}
}

#[derive(Debug, PartialEq)]
struct ImageViewOwner<'a>(*mut ZXing_ImageView, PhantomData<&'a u8>);

impl Drop for ImageViewOwner<'_> {
	fn drop(&mut self) {
		unsafe { ZXing_ImageView_delete(self.0) }
	}
}
#[derive(Debug, Clone, PartialEq)]
pub struct ImageView<'a>(Rc<ImageViewOwner<'a>>);

impl<'a> From<&'a ImageView<'a>> for ImageView<'a> {
	fn from(img: &'a ImageView) -> Self {
		img.clone()
	}
}

impl<'a> ImageView<'a> {
	fn try_into_int<T: TryInto<c_int>>(val: T) -> Result<c_int, Error> {
		val.try_into().map_err(|_| Error::InvalidInput("Could not convert Integer into c_int.".to_string()))
	}

	/// Constructs an ImageView from a raw pointer and the width/height (in pixels)
	/// and row_stride/pix_stride (in bytes).
	///
	/// # Safety
	///
	/// The memory gets accessed inside the c++ library at random places between
	/// `ptr` and `ptr + height * row_stride` or `ptr + width * pix_stride`.
	/// Note that both the stride values could be negative, e.g. if the image
	/// view is rotated.
	pub unsafe fn from_ptr<T: TryInto<c_int>, U: TryInto<c_int>>(
		ptr: *const u8,
		width: T,
		height: T,
		format: ImageFormat,
		row_stride: U,
		pix_stride: U,
	) -> Result<Self, Error> {
		let iv = ZXing_ImageView_new(
			ptr,
			Self::try_into_int(width)?,
			Self::try_into_int(height)?,
			format as ZXing_ImageFormat,
			Self::try_into_int(row_stride)?,
			Self::try_into_int(pix_stride)?,
		);
		last_error_if_null_or!(iv, ImageView(Rc::new(ImageViewOwner(iv, PhantomData))))
	}

	pub fn from_slice<T: TryInto<c_int>>(data: &'a [u8], width: T, height: T, format: ImageFormat) -> Result<Self, Error> {
		unsafe {
			let iv = ZXing_ImageView_new_checked(
				data.as_ptr(),
				data.len() as c_int,
				Self::try_into_int(width)?,
				Self::try_into_int(height)?,
				format as ZXing_ImageFormat,
				0,
				0,
			);
			last_error_if_null_or!(iv, ImageView(Rc::new(ImageViewOwner(iv, PhantomData))))
		}
	}

	pub fn cropped(self, left: i32, top: i32, width: i32, height: i32) -> Self {
		unsafe { ZXing_ImageView_crop((self.0).0, left, top, width, height) }
		self
	}

	pub fn rotated(self, degree: i32) -> Self {
		unsafe { ZXing_ImageView_rotate((self.0).0, degree) }
		self
	}
}

#[cfg(feature = "image")]
use image;

#[cfg(feature = "image")]
impl<'a> From<&'a image::GrayImage> for ImageView<'a> {
	fn from(img: &'a image::GrayImage) -> Self {
		ImageView::from_slice(img.as_ref(), img.width(), img.height(), ImageFormat::Lum).unwrap()
	}
}

#[cfg(feature = "image")]
impl<'a> TryFrom<&'a image::DynamicImage> for ImageView<'a> {
	type Error = Error;

	fn try_from(img: &'a image::DynamicImage) -> Result<Self, Error> {
		let format = match img {
			image::DynamicImage::ImageLuma8(_) => Some(ImageFormat::Lum),
			image::DynamicImage::ImageLumaA8(_) => Some(ImageFormat::LumA),
			image::DynamicImage::ImageRgb8(_) => Some(ImageFormat::RGB),
			image::DynamicImage::ImageRgba8(_) => Some(ImageFormat::RGBA),
			_ => None,
		};
		match format {
			Some(format) => Ok(ImageView::from_slice(img.as_bytes(), img.width(), img.height(), format)?),
			None => Err(Error::InvalidInput("Invalid image format (must be either luma8|lumaA8|rgb8|rgba8)".to_string())),
		}
	}
}

make_zxing_class!(Image, ZXing_Image);

impl Image {
	getter!(Image, width, transmute, i32);
	getter!(Image, height, transmute, i32);
	getter!(Image, format, transmute, ImageFormat);

	pub fn data(&self) -> Vec<u8> {
		let ptr = unsafe { ZXing_Image_data(self.0) };
		if ptr.is_null() {
			Vec::<u8>::new()
		} else {
			unsafe { std::slice::from_raw_parts(ptr, (self.width() * self.height()) as usize).to_vec() }
		}
	}
}

#[cfg(feature = "image")]
impl From<&Image> for image::GrayImage {
	fn from(img: &Image) -> image::GrayImage {
		image::GrayImage::from_vec(img.width() as u32, img.height() as u32, img.data()).unwrap()
	}
}

#[derive(Error, Debug, PartialEq)]
pub enum BarcodeError {
	#[error("")]
	None(),

	#[error("{0}")]
	Checksum(String),

	#[error("{0}")]
	Format(String),

	#[error("{0}")]
	Unsupported(String),
}

pub type PointI = ZXing_PointI;
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct Position {
	pub top_left: PointI,
	pub top_right: PointI,
	pub bottom_right: PointI,
	pub bottom_left: PointI,
}

impl Display for PointI {
	fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
		write!(f, "{}x{}", self.x, self.y)
	}
}

impl Display for Position {
	fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
		write!(f, "{}", unsafe {
			c2r_str(ZXing_PositionToString(*(self as *const Position as *const ZXing_Position)))
		})
	}
}

make_zxing_class!(Barcode, ZXing_Barcode);

impl Barcode {
	getter!(Barcode, isValid, transmute, bool);
	getter!(Barcode, format, (|f| BarcodeFormats::new(f).unwrap().into_iter().last().unwrap()), BarcodeFormat);
	getter!(Barcode, contentType, transmute, ContentType);
	getter!(Barcode, text, c2r_str, String);
	getter!(Barcode, ecLevel, c2r_str, String);
	getter!(Barcode, symbologyIdentifier, c2r_str, String);
	getter!(Barcode, position, transmute, Position);
	getter!(Barcode, orientation, transmute, i32);
	getter!(Barcode, hasECI, has_eci, transmute, bool);
	getter!(Barcode, isInverted, transmute, bool);
	getter!(Barcode, isMirrored, transmute, bool);
	getter!(Barcode, lineCount, transmute, i32);

	pub fn bytes(&self) -> Vec<u8> {
		let mut len: c_int = 0;
		unsafe { c2r_vec(ZXing_Barcode_bytes(self.0, &mut len), len) }
	}
	pub fn bytes_eci(&self) -> Vec<u8> {
		let mut len: c_int = 0;
		unsafe { c2r_vec(ZXing_Barcode_bytesECI(self.0, &mut len), len) }
	}

	pub fn error(&self) -> BarcodeError {
		let error_type = unsafe { ZXing_Barcode_errorType(self.0) };
		let error_msg = unsafe { c2r_str(ZXing_Barcode_errorMsg(self.0)) };
		#[allow(non_upper_case_globals)]
		match error_type {
			ZXing_ErrorType_None => BarcodeError::None(),
			ZXing_ErrorType_Format => BarcodeError::Format(error_msg),
			ZXing_ErrorType_Checksum => BarcodeError::Checksum(error_msg),
			ZXing_ErrorType_Unsupported => BarcodeError::Unsupported(error_msg),
			_ => panic!("Internal error: invalid ZXing_ErrorType"),
		}
	}

	pub fn to_svg_with(&self, opts: &BarcodeWriter) -> Result<String, Error> {
		let str = unsafe { ZXing_WriteBarcodeToSVG(self.0, opts.0) };
		last_error_if_null_or!(str, c2r_str(str))
	}

	pub fn to_svg(&self) -> Result<String, Error> {
		self.to_svg_with(&BarcodeWriter::default())
	}

	pub fn to_image_with(&self, opts: &BarcodeWriter) -> Result<Image, Error> {
		let img = unsafe { ZXing_WriteBarcodeToImage(self.0, opts.0) };
		last_error_if_null_or!(img, Image(img))
	}

	pub fn to_image(&self) -> Result<Image, Error> {
		self.to_image_with(&BarcodeWriter::default())
	}
}

make_zxing_class_with_default!(BarcodeReader, ZXing_ReaderOptions);

impl BarcodeReader {
	property!(ReaderOptions, TryHarder, bool);
	property!(ReaderOptions, TryRotate, bool);
	property!(ReaderOptions, TryInvert, bool);
	property!(ReaderOptions, TryDownscale, bool);
	property!(ReaderOptions, IsPure, bool);
	property!(ReaderOptions, ReturnErrors, bool);
	property!(ReaderOptions, Formats, BarcodeFormats);
	property!(ReaderOptions, TextMode, TextMode);
	property!(ReaderOptions, Binarizer, Binarizer);
	property!(ReaderOptions, EanAddOnSymbol, EanAddOnSymbol);
	property!(ReaderOptions, MaxNumberOfSymbols, i32);
	property!(ReaderOptions, MinLineCount, i32);

	pub fn from<'a, IV>(&self, image: IV) -> Result<Vec<Barcode>, Error>
	where
		IV: TryInto<ImageView<'a>>,
		IV::Error: Into<Error>,
	{
		let iv_: ImageView = image.try_into().map_err(Into::into)?;
		unsafe {
			let results = ZXing_ReadBarcodes((iv_.0).0, self.0);
			if !results.is_null() {
				let size = ZXing_Barcodes_size(results);
				let mut vec = Vec::<Barcode>::with_capacity(size as usize);
				for i in 0..size {
					vec.push(Barcode(ZXing_Barcodes_move(results, i)));
				}
				ZXing_Barcodes_delete(results);
				Ok(vec)
			} else {
				Err(last_error())
			}
		}
	}
}

make_zxing_class!(BarcodeCreator, ZXing_CreatorOptions);

impl BarcodeCreator {
	pub fn new(format: BarcodeFormat) -> Self {
		unsafe { BarcodeCreator(ZXing_CreatorOptions_new(BarcodeFormats::from(format).bits())) }
	}

	property!(CreatorOptions, ReaderInit, bool);
	property!(CreatorOptions, ForceSquareDataMatrix, bool);
	property!(CreatorOptions, EcLevel, String);

	pub fn from_str(&self, str: impl AsRef<str>) -> Result<Barcode, Error> {
		let cstr = CString::new(str.as_ref())?;
		let bc = unsafe { ZXing_CreateBarcodeFromText(cstr.as_ptr(), 0, self.0) };
		last_error_if_null_or!(bc, Barcode(bc))
	}

	pub fn from_slice(&self, data: impl AsRef<[u8]>) -> Result<Barcode, Error> {
		let data = data.as_ref();
		let bc = unsafe { ZXing_CreateBarcodeFromBytes(data.as_ptr() as *const c_void, data.len() as i32, self.0) };
		last_error_if_null_or!(bc, Barcode(bc))
	}
}

make_zxing_class_with_default!(BarcodeWriter, ZXing_WriterOptions);

impl BarcodeWriter {
	property!(WriterOptions, Scale, i32);
	property!(WriterOptions, SizeHint, i32);
	property!(WriterOptions, Rotate, i32);
	property!(WriterOptions, WithHRT, with_hrt, bool);
	property!(WriterOptions, WithQuietZones, bool);
}

pub fn read() -> BarcodeReader {
	BarcodeReader::default()
}

pub fn create(format: BarcodeFormat) -> BarcodeCreator {
	BarcodeCreator::new(format)
}

pub fn write() -> BarcodeWriter {
	BarcodeWriter::default()
}

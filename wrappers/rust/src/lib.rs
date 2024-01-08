#![feature(arbitrary_self_types)]
#![allow(unused_imports)]

mod barcode_result;
mod image_view;
mod reader_options;

pub use barcode_result::*;
pub use image_view::*;
pub use reader_options::*;

mod bindings;

pub use bindings::base_ffi::BarcodeFormat;
pub use bindings::base_ffi::CharacterSet;
pub use bindings::base_ffi::ContentType;
pub use bindings::base_ffi::ImageFormat;
pub use bindings::base_ffi::TextMode;
pub use bindings::wrapped_ffi::Binarizer;
pub use bindings::wrapped_ffi::EanAddOnSymbol;

use autocxx::prelude::*;
use std::fmt::{Display, Formatter};
use std::io::Cursor;
use std::pin::Pin;

/// Read barcode from an [ImageView]
///
/// * `image` - view of the image data including layout and format
/// * `options` - [ReaderOptions] to parameterize / speed up detection
///
/// Returns a [BarcodeResult]
pub fn read_barcode(image: ImageView, mut reader_options: ReaderOptions) -> BarcodeResult {
    BarcodeResult {
        result: bindings::base_ffi::ReadBarcode(&image.image, unsafe {
            reader_options.options.as_cpp_mut_ref().asOptions().as_mut()
        })
        .within_unique_ptr(),
    }
}

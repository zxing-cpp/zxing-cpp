#![allow(unused_imports)]

mod barcode_format;
mod barcode_result;
mod image_view;
mod reader_options;

pub use barcode_format::*;
pub use barcode_result::*;
pub use image_view::*;
pub use reader_options::*;

mod bindings;

pub use bindings::base_ffi::Binarizer;
pub use bindings::base_ffi::CharacterSet;
pub use bindings::base_ffi::ContentType;
pub use bindings::base_ffi::EanAddOnSymbol;
pub use bindings::base_ffi::ImageFormat;
pub use bindings::base_ffi::TextMode;

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
pub fn read_barcodes(image: &ImageView, reader_options: &ReaderOptions) -> Results {
    Results {
        results: bindings::base_ffi::readBarcodes(&image.image, &reader_options.options)
            .within_unique_ptr(),
    }
}

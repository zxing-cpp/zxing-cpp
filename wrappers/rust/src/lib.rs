#![feature(arbitrary_self_types)]
#![allow(unused_imports)]

mod barcode_format;
mod barcode_result;
mod binarizer;
mod character_set;
mod content_type;
mod ean_add_on_symbol;
mod image_format;
mod image_view;
mod multi_format_writer;
mod reader_options;
mod text_mode;

mod matrix;

pub use barcode_format::*;
pub use barcode_result::*;
pub use binarizer::*;
pub use character_set::*;
pub use content_type::*;
pub use ean_add_on_symbol::*;
pub use image_format::*;
pub use image_view::*;
pub use matrix::*;
pub use multi_format_writer::*;
pub use reader_options::*;
pub use text_mode::*;

mod bindings;

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

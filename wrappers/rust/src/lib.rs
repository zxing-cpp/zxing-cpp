#![feature(arbitrary_self_types)]
#![allow(unused_imports)]

mod barcode_format;
mod barcode_result;
mod image_format;
mod image_view;
mod reader_options;

pub use barcode_format::*;
pub use barcode_result::*;
pub use image_format::*;
pub use image_view::*;
pub use reader_options::*;

mod binarizer;
mod bindings;
mod content_type;
mod ean_add_on_symbol;
mod text_mode;

use autocxx::prelude::*;
use std::fmt::{Display, Formatter};
use std::io::Cursor;
use std::pin::Pin;

pub fn read_barcode(image: ImageView, mut reader_options: ReaderOptions) -> BarcodeResult {
    BarcodeResult {
        result: bindings::base_ffi::ReadBarcode(&image.image, unsafe {
            reader_options.options.as_cpp_mut_ref().asOptions().as_mut()
        })
        .within_unique_ptr(),
    }
}

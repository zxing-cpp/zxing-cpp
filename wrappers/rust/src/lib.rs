mod bindings;

pub use bindings::ffi::CharacterSet;

pub use bindings::ffi::ImageFormat;

pub use bindings::ffi::Binarizer;
pub use bindings::ffi::EanAddOnSymbol;
pub use bindings::ffi::TextMode;

mod barcode_format;
mod image_view;
mod reader_options;
mod results;

pub use barcode_format::*;
pub use image_view::*;
pub use reader_options::*;
pub use results::*;

pub fn read_barcodes(image: &ImageView, options: &ReaderOptions) -> Result<Results, String> {
    let results = bindings::ffi::read_barcodes(&image.image, &options.options)
        .map_err(|e| e.what().to_string())?;
    Ok(Results { results })
}

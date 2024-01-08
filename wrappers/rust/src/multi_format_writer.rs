use crate::matrix::Matrix;
use crate::{bindings, BarcodeFormat, CharacterSet};
use autocxx::{c_int, AsCppMutRef, CppUniquePtrPin, WithinUniquePtr, WithinUniquePtrTrivial};
use cxx::{let_cxx_string, UniquePtr};

/// Single-point service to generate barcodes for all supported formats.
///
/// This struct is here for convenience. As a result, this class offers very limited customization
/// compared ot what are available in each individual encoder.
pub struct MultiFormatWriter {
    writer: UniquePtr<bindings::base_ffi::MultiFormatWriterExt>,
}

impl MultiFormatWriter {
    /// Creates a new [MultiFormatWriter] with a specified format to encode data.
    pub fn new(format: BarcodeFormat) -> Self {
        Self {
            writer: bindings::base_ffi::MultiFormatWriterExt::new(format.into())
                .within_unique_ptr(),
        }
    }

    /// Used for Aztec, PDF417, and QRCode only.
    pub fn encoding(&mut self, encoding: CharacterSet) {
        self.writer
            .pin_mut()
            .asBase()
            .setEncoding(bindings::base_ffi::CharacterSet::from(encoding).within_unique_ptr());
    }

    /// Used for Aztec, PDF417, and QRCode only, \[0-8\].
    pub fn ecc_level(&mut self, level: u32) {
        self.writer
            .pin_mut()
            .asBase()
            .setEccLevel(c_int(level as i32));
    }

    /// Used for all formats, sets the minimum number of quiet zone pixels.
    pub fn margin(&mut self, margin: u32) {
        self.writer
            .pin_mut()
            .asBase()
            .setMargin(c_int(margin as i32));
    }

    /// Encodes `contents` into a matrix that holds the image data for the created barcode
    ///
    /// * `contents` - data to encode
    /// * `width` - width of the created barcode
    /// * `height` - height of the created barcode
    pub fn encode_to_matrix(&self, contents: &str, width: u32, height: u32) -> Matrix {
        let_cxx_string!(contents = contents);
        self.writer
            .encodeToMatrix(&contents, c_int(width as i32), c_int(height as i32))
            .within_unique_ptr()
            .into()
    }
}

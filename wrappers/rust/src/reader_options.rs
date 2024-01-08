use crate::binarizer::Binarizer;
use crate::ean_add_on_symbol::EanAddOnSymbol;
use crate::text_mode::TextMode;
use crate::{bindings, BarcodeFormat};
use autocxx::{AsCppMutRef, AsCppRef, CppUniquePtrPin, WithinUniquePtr, WithinUniquePtrTrivial};
use cxx::UniquePtr;

/// Struct that stores parameters used for reading barcodes ([read_barcode])
///
/// [read_barcode]: crate::read_barcode
pub struct ReaderOptions {
    pub(crate) options: CppUniquePtrPin<bindings::wrapped_ffi::ReaderOptionsExt>,
}

impl Default for ReaderOptions {
    fn default() -> Self {
        Self {
            options: CppUniquePtrPin::new(
                bindings::wrapped_ffi::ReaderOptionsExt::new().within_unique_ptr(),
            ),
        }
    }
}

impl ReaderOptions {
    /// Specify a set of BarcodeFormats that should be searched for.
    ///
    /// The default is all supported formats.
    pub fn add_format(mut self, f: BarcodeFormat) -> Self {
        self.options
            .as_cpp_mut_ref()
            .addFormat(bindings::base_ffi::BarcodeFormat::from(f).within_unique_ptr());
        self
    }

    /// Spend more time to try to find a barcode; optimize for accuracy, not speed.
    pub fn try_harder(mut self, try_harder: bool) -> Self {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setTryHarder(try_harder);
        self
    }

    /// Also try detecting code in 90, 180, and 270 degree rotated images.
    pub fn try_rotate(mut self, try_rotate: bool) -> Self {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setTryRotate(try_rotate);
        self
    }

    /// Also try detecting inverted ("reversed reflectance") codes if the format allows for those.
    pub fn try_invert(mut self, try_invert: bool) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setTryInvert(try_invert);
    }

    /// Also try detecting code in downscaled images (depending on image size).
    pub fn try_downscale(mut self, try_downscale: bool) -> Self {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setTryDownscale(try_downscale);
        self
    }

    /// Set to true if the input contains nothing but a single perfectly aligned barcode (generated image).
    pub fn pure(mut self, pure: bool) -> Self {
        self.options.as_cpp_mut_ref().asOptions().setIsPure(pure);
        self
    }

    /// If true, return the barcodes with errors as well (e.g. checksum errors)
    pub fn return_errors(mut self, return_errors: bool) -> Self {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setReturnErrors(return_errors);

        self
    }

    /// [Binarizer] to use internally when using the [read_barcode] function
    ///
    /// [read_barcode]: crate::read_barcode
    pub fn binarizer(mut self, binarizer: Binarizer) -> Self {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setBinarizer(binarizer.into());
        self
    }

    /// Specify whether to ignore, read or require EAN-2/5 add-on symbols while scanning EAN/UPC codes
    pub fn ean_add_on_symbol(&mut self, ean_add_on_symbol: EanAddOnSymbol) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setEanAddOnSymbol(ean_add_on_symbol.into());
    }

    /// Specifies the TextMode that controls the return of the [BarcodeResult::text] function
    ///
    /// [BarcodeResult::text]: crate::BarcodeResult::text
    pub fn text_mode(&mut self, text_mode: TextMode) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setTextMode(text_mode.into());
    }
}

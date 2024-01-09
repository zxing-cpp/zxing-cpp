use crate::{bindings, BarcodeFormat, Binarizer, EanAddOnSymbol, TextMode};
use autocxx::{
    c_int, AsCppMutRef, AsCppRef, CppUniquePtrPin, WithinUniquePtr, WithinUniquePtrTrivial,
};
use cxx::UniquePtr;
use flagset::FlagSet;

/// Struct that stores parameters used for reading barcodes ([read_barcode])
///
/// [read_barcode]: crate::read_barcode
pub struct ReaderOptions {
    pub(crate) options: UniquePtr<bindings::base_ffi::ReaderOptionsExt>,
}

impl Default for ReaderOptions {
    fn default() -> Self {
        Self {
            options: bindings::base_ffi::ReaderOptionsExt::new().within_unique_ptr(),
        }
    }
}

impl ReaderOptions {
    pub fn set_formats(mut self, formats: FlagSet<BarcodeFormat>) -> Self {
        self.options
            .pin_mut()
            .setFormats(c_int(formats.bits() as i32));
        self
    }

    /// Spend more time to try to find a barcode; optimize for accuracy, not speed.
    pub fn try_harder(mut self, try_harder: bool) -> Self {
        self.options.pin_mut().tryHarder(try_harder);
        self
    }

    /// Also try detecting code in 90, 180, and 270 degree rotated images.
    pub fn try_rotate(mut self, try_rotate: bool) -> Self {
        self.options.pin_mut().tryRotate(try_rotate);
        self
    }

    /// Also try detecting inverted ("reversed reflectance") codes if the format allows for those.
    pub fn try_invert(mut self, try_invert: bool) {
        self.options.pin_mut().tryInvert(try_invert);
    }

    /// Also try detecting code in downscaled images (depending on image size).
    pub fn try_downscale(mut self, try_downscale: bool) -> Self {
        self.options.pin_mut().tryDownscale(try_downscale);
        self
    }

    ///  to true if the input contains nothing but a single perfectly aligned barcode (generated image).
    pub fn pure(mut self, pure: bool) -> Self {
        self.options.pin_mut().pure(pure);
        self
    }

    /// If true, return the barcodes with errors as well (e.g. checksum errors)
    pub fn return_errors(mut self, return_errors: bool) -> Self {
        self.options.pin_mut().returnErrors(return_errors);

        self
    }

    /// [Binarizer] to use internally when using the [read_barcode] function
    ///
    /// [read_barcode]: crate::read_barcode
    pub fn binarizer(mut self, binarizer: Binarizer) -> Self {
        self.options.pin_mut().binarizer(binarizer);
        self
    }

    /// Specify whether to ignore, read or require EAN-2/5 add-on symbols while scanning EAN/UPC codes
    pub fn ean_add_on_symbol(&mut self, ean_add_on_symbol: EanAddOnSymbol) {
        self.options.pin_mut().eanAddOnSymbol(ean_add_on_symbol);
    }

    /// Specifies the TextMode that controls the return of the [BarcodeResult::text] function
    ///
    /// [BarcodeResult::text]: crate::BarcodeResult::text
    pub fn text_mode(&mut self, text_mode: TextMode) {
        self.options.pin_mut().textMode(text_mode);
    }
}

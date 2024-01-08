use crate::binarizer::Binarizer;
use crate::ean_add_on_symbol::EanAddOnSymbol;
use crate::text_mode::TextMode;
use crate::{bindings, BarcodeFormat};
use autocxx::{AsCppMutRef, AsCppRef, CppUniquePtrPin, WithinUniquePtr, WithinUniquePtrTrivial};
use cxx::UniquePtr;

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
    pub fn add_format(&mut self, f: BarcodeFormat) {
        self.options
            .as_cpp_mut_ref()
            .addFormat(bindings::base_ffi::BarcodeFormat::from(f).within_unique_ptr());
    }

    pub fn try_harder(&mut self, try_harder: bool) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setTryHarder(try_harder);
    }

    pub fn try_rotate(&mut self, try_rotate: bool) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setTryRotate(try_rotate);
    }

    pub fn try_invert(&mut self, try_invert: bool) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setTryInvert(try_invert);
    }

    pub fn try_downscale(&mut self, try_downscale: bool) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setTryDownscale(try_downscale);
    }

    pub fn pure(&mut self, pure: bool) {
        self.options.as_cpp_mut_ref().asOptions().setIsPure(pure);
    }

    pub fn return_errors(&mut self, return_errors: bool) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setReturnErrors(return_errors);
    }

    pub fn binarizer(&mut self, binarizer: Binarizer) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setBinarizer(binarizer.into());
    }

    pub fn ean_add_on_symbol(&mut self, ean_add_on_symbol: EanAddOnSymbol) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setEanAddOnSymbol(ean_add_on_symbol.into());
    }

    pub fn text_mode(&mut self, text_mode: TextMode) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setTextMode(text_mode.into());
    }

    pub fn max_number_of_symbols(&mut self, n: u8) {
        self.options
            .as_cpp_mut_ref()
            .asOptions()
            .setMaxNumberOfSymbols(n);
    }
}

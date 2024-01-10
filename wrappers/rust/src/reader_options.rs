use crate::{bindings, BarcodeFormat, Binarizer, EanAddOnSymbol, TextMode};
use cxx::UniquePtr;
use flagset::FlagSet;

pub struct ReaderOptions {
    pub(crate) options: UniquePtr<bindings::ffi::ReaderOptionsExt>,
}

impl Default for ReaderOptions {
    fn default() -> Self {
        Self {
            options: bindings::ffi::new_reader_options(),
        }
    }
}

impl ReaderOptions {
    pub fn formats(mut self, formats: impl Into<FlagSet<BarcodeFormat>>) -> Self {
        self.options
            .pin_mut()
            .set_formats(formats.into().bits() as i32);
        self
    }

    pub fn try_harder(mut self, try_harder: bool) -> Self {
        self.options.pin_mut().try_harder(try_harder);
        self
    }

    pub fn try_rotate(mut self, try_rotate: bool) -> Self {
        self.options.pin_mut().try_rotate(try_rotate);
        self
    }

    pub fn try_invert(mut self, try_invert: bool) -> Self {
        self.options.pin_mut().try_invert(try_invert);
        self
    }

    pub fn try_downscale(mut self, try_downscale: bool) -> Self {
        self.options.pin_mut().try_downscale(try_downscale);
        self
    }

    pub fn pure(mut self, pure: bool) -> Self {
        self.options.pin_mut().pure(pure);
        self
    }

    pub fn return_errors(mut self, return_errors: bool) -> Self {
        self.options.pin_mut().return_errors(return_errors);
        self
    }

    pub fn binarizer(mut self, binarizer: Binarizer) -> Self {
        self.options.pin_mut().binarizer(binarizer);
        self
    }

    pub fn ean_add_on_symbol(mut self, ean_add_on_symbol: EanAddOnSymbol) -> Self {
        self.options.pin_mut().ean_add_on_symbol(ean_add_on_symbol);
        self
    }

    pub fn text_mode(mut self, text_mode: TextMode) -> Self {
        self.options.pin_mut().text_mode(text_mode);
        self
    }
}

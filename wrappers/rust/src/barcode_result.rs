use crate::content_type::ContentType;
use crate::{bindings, BarcodeFormat};
use autocxx::WithinUniquePtrTrivial;
use cxx::UniquePtr;

pub struct BarcodeResult {
    pub(crate) result: UniquePtr<bindings::base_ffi::Result>,
}

impl BarcodeResult {
    pub fn format(&self) -> BarcodeFormat {
        self.result.format().into()
    }

    pub fn text(&self) -> String {
        self.result.text1().to_string()
    }

    pub fn is_valid(&self) -> bool {
        self.result.isValid()
    }

    pub fn error_message(&self) -> String {
        bindings::base_ffi::ErrorToString(self.result.error()).to_string()
    }

    pub fn content_type(&self) -> ContentType {
        self.result.contentType().into()
    }

    pub fn bytes(&self) -> &[u8] {
        bindings::base_ffi::ByteArrayAsVec(self.result.bytes()).as_slice()
    }

    pub fn ec_level(&self) -> String {
        self.result.ecLevel().to_string()
    }

    pub fn symbology_identifier(&self) -> String {
        self.result.symbologyIdentifier().to_string()
    }

    pub fn orientation(&self) -> u32 {
        self.result.orientation().0 as u32
    }

    pub fn is_inverted(&self) -> bool {
        self.result.isInverted()
    }

    pub fn is_mirrored(&self) -> bool {
        self.result.isMirrored()
    }
}

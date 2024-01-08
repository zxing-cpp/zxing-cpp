use crate::content_type::ContentType;
use crate::{bindings, BarcodeFormat};
use autocxx::WithinUniquePtrTrivial;
use cxx::UniquePtr;

/// Encapsulates the result of decoding a barcode within an image.
pub struct BarcodeResult {
    pub(crate) result: UniquePtr<bindings::base_ffi::Result>,
}

impl BarcodeResult {
    /// The format of the decoded barcode
    pub fn format(&self) -> BarcodeFormat {
        self.result.format().into()
    }

    /// The contents of [BarcodeResult::bytes] rendered to unicode/utf8 text according to the [TextMode] set in [ReaderOptions]
    ///
    /// [TextMode]: crate::TextMode
    /// [ReaderOptions]: crate::ReaderOptions
    pub fn text(&self) -> String {
        self.result.text1().to_string()
    }

    /// If the result is valid
    pub fn is_valid(&self) -> bool {
        self.result.isValid()
    }

    /// Text of the internal error
    pub fn error_message(&self) -> String {
        bindings::base_ffi::ErrorToString(self.result.error()).to_string()
    }

    /// Gives a hint to the type of content found (Text/Binary/GS1/etc.)
    pub fn content_type(&self) -> ContentType {
        self.result.contentType().into()
    }

    /// The raw / standard content without any modifications like character set conversions
    pub fn bytes(&self) -> &[u8] {
        bindings::base_ffi::ByteArrayAsVec(self.result.bytes()).as_slice()
    }

    /// The error correction level of the symbol (empty string if not applicable)
    pub fn ec_level(&self) -> String {
        self.result.ecLevel().to_string()
    }

    /// Symbology identifier "]cm" where "c" is symbology code character, "m" the modifier.
    pub fn symbology_identifier(&self) -> String {
        self.result.symbologyIdentifier().to_string()
    }

    /// Orientation of the barcode in degree
    pub fn orientation(&self) -> u32 {
        self.result.orientation().0 as u32
    }

    /// If the symbol is inverted / has reversed reflectance
    pub fn is_inverted(&self) -> bool {
        self.result.isInverted()
    }

    /// If the symbol is mirrored (currently only supported by QRCode and DataMatrix)
    pub fn is_mirrored(&self) -> bool {
        self.result.isMirrored()
    }
}

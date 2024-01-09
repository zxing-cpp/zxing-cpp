use crate::{bindings, BarcodeFormat, ContentType};
use autocxx::WithinUniquePtrTrivial;
use autocxx::{c_int, WithinUniquePtr};
use cxx::UniquePtr;
use std::ops::Deref;

/// Struct containing the results of reading barcodes
pub struct Results {
    pub(crate) results: UniquePtr<bindings::base_ffi::ResultsExt>,
}

impl Results {
    /// The length of the list of results
    pub fn len(&self) -> usize {
        self.results.size().0 as usize
    }

    /// If the list of results is empty
    pub fn is_empty(&self) -> bool {
        self.results.size().0 == 0
    }
}

impl<'a> IntoIterator for &'a Results {
    type Item = BarcodeResult<'a>;
    type IntoIter = ResultIterator<'a>;

    fn into_iter(self) -> Self::IntoIter {
        ResultIterator {
            results: self,
            index: 0,
        }
    }
}

/// Iterator over the contents of [Results]
pub struct ResultIterator<'a> {
    results: &'a Results,
    index: u32,
}

impl<'a> Iterator for ResultIterator<'a> {
    type Item = BarcodeResult<'a>;

    fn next(&mut self) -> Option<Self::Item> {
        let res = if self.index < self.results.results.size().0 as u32 {
            Some(self.results.results.at(c_int(self.index as i32)).into())
        } else {
            None
        };
        self.index += 1;
        res
    }
}

/// Encapsulates the result of decoding a barcode within an image.
pub struct BarcodeResult<'a> {
    pub(crate) result: &'a bindings::base_ffi::Result,
}

impl<'a> From<&'a bindings::base_ffi::Result> for BarcodeResult<'a> {
    fn from(value: &'a bindings::base_ffi::Result) -> Self {
        Self { result: value }
    }
}

impl<'a> BarcodeResult<'a> {
    /// The format of the decoded barcode
    pub fn format(&self) -> BarcodeFormat {
        (self.result.format() as u32).into()
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
        bindings::base_ffi::errorToString(self.result.error()).to_string()
    }

    /// Gives a hint to the type of content found (Text/Binary/GS1/etc.)
    pub fn content_type(&self) -> ContentType {
        self.result.contentType()
    }

    /// The raw / standard content without any modifications like character set conversions
    pub fn bytes(&self) -> &[u8] {
        bindings::base_ffi::byteArrayAsVec(self.result.bytes()).as_slice()
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

use crate::bindings::ffi::ContentType;
use crate::{bindings, BarcodeFormat};
use cxx::{CxxVector, UniquePtr};

pub struct Results {
    pub(crate) results: UniquePtr<CxxVector<bindings::ffi::Result>>,
}

impl Results {
    /// The length of the list of results
    pub fn len(&self) -> usize {
        self.results.len()
    }

    /// If the list of results is empty
    pub fn is_empty(&self) -> bool {
        self.results.is_empty()
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
    index: usize,
}

impl<'a> Iterator for ResultIterator<'a> {
    type Item = BarcodeResult<'a>;

    fn next(&mut self) -> Option<Self::Item> {
        let res = self
            .results
            .results
            .get(self.index)
            .map(|result| result.into());
        self.index += 1;
        res
    }
}

/// Encapsulates the result of decoding a barcode within an image.
pub struct BarcodeResult<'a> {
    pub(crate) result: &'a bindings::ffi::Result,
}

impl<'a> From<&'a bindings::ffi::Result> for BarcodeResult<'a> {
    fn from(value: &'a bindings::ffi::Result) -> Self {
        Self { result: value }
    }
}

impl<'a> BarcodeResult<'a> {
    /// The format of the decoded barcode
    pub fn format(&self) -> BarcodeFormat {
        BarcodeFormat::from(bindings::ffi::format_of_result(self.result) as u32)
    }

    /// The contents of [BarcodeResult::bytes] rendered to unicode/utf8 text according to the [TextMode] set in [ReaderOptions]
    ///
    /// [TextMode]: crate::TextMode
    /// [ReaderOptions]: crate::ReaderOptions
    pub fn text(&self) -> String {
        bindings::ffi::text_of_result(self.result)
    }

    /// If the result is valid
    pub fn is_valid(&self) -> bool {
        self.result.is_valid()
    }

    /// Text of the internal error
    pub fn error_message(&self) -> String {
        bindings::ffi::error_to_string(self.result.error())
    }

    /// Gives a hint to the type of content found (Text/Binary/GS1/etc.)
    pub fn content_type(&self) -> ContentType {
        self.result.content_type()
    }

    /// The raw / standard content without any modifications like character set conversions
    pub fn bytes(&self) -> &[u8] {
        bindings::ffi::byte_array_as_vec(self.result.bytes()).as_slice()
    }

    /// The error correction level of the symbol (empty string if not applicable)
    pub fn ec_level(&self) -> String {
        bindings::ffi::ec_level_of_result(self.result)
    }

    /// Symbology identifier "]cm" where "c" is symbology code character, "m" the modifier.
    pub fn symbology_identifier(&self) -> String {
        bindings::ffi::symbology_identifier_of_result(self.result)
    }

    /// Orientation of the barcode in degree
    pub fn orientation(&self) -> u32 {
        self.result.orientation() as u32
    }

    /// If the symbol is inverted / has reversed reflectance
    pub fn is_inverted(&self) -> bool {
        self.result.is_inverted()
    }

    /// If the symbol is mirrored (currently only supported by QRCode and DataMatrix)
    pub fn is_mirrored(&self) -> bool {
        self.result.is_mirrored()
    }
}

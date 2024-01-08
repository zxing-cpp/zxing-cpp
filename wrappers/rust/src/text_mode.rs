#![allow(clippy::upper_case_acronyms)]

use crate::bindings;

/// Enum representing how to transcode the contents of a read barcode
#[derive(Copy, Clone)]
pub enum TextMode {
    /// [BarcodeResult::bytes] transcoded to unicode based on ECI info or guessed charset (the default mode prior to zxing-cpp 2.0)
    ///
    /// [BarcodeResult::bytes]: crate::BarcodeResult::bytes
    Plain,
    /// Standard content following the ECI protocol with every character set ECI segment transcoded to unicode
    ECI,
    /// Human Readable Interpretation (dependent on the [ContentType])
    ///
    /// [ContentType]: crate::ContentType
    HRI,
    /// [BarcodeResult::bytes] transcoded to ASCII string of HEX values
    ///
    /// [BarcodeResult::bytes]: crate::BarcodeResult::bytes
    Hex,
    /// Use the EscapeNonGraphical() function (e.g. ASCII 29 will be transcoded to "\<GS\>")
    Escaped,
}

impl From<bindings::wrapped_ffi::TextMode> for TextMode {
    fn from(value: bindings::wrapped_ffi::TextMode) -> Self {
        use bindings::wrapped_ffi::TextMode as TM;
        match value {
            TM::Plain => TextMode::Plain,
            TM::ECI => TextMode::ECI,
            TM::HRI => TextMode::HRI,
            TM::Hex => TextMode::Hex,
            TM::Escaped => TextMode::Escaped,
        }
    }
}

impl From<TextMode> for bindings::wrapped_ffi::TextMode {
    fn from(value: TextMode) -> Self {
        use bindings::wrapped_ffi::TextMode as TM;
        match value {
            TextMode::Plain => TM::Plain,
            TextMode::ECI => TM::ECI,
            TextMode::HRI => TM::HRI,
            TextMode::Hex => TM::Hex,
            TextMode::Escaped => TM::Escaped,
        }
    }
}

#![allow(clippy::upper_case_acronyms)]

use crate::bindings;

#[derive(Copy, Clone)]
pub enum TextMode {
    Plain,
    ECI,
    HRI,
    Hex,
    Escaped,
}

impl From<bindings::reader_options_ffi::TextMode> for TextMode {
    fn from(value: bindings::reader_options_ffi::TextMode) -> Self {
        use bindings::reader_options_ffi::TextMode as TM;
        match value {
            TM::Plain => TextMode::Plain,
            TM::ECI => TextMode::ECI,
            TM::HRI => TextMode::HRI,
            TM::Hex => TextMode::Hex,
            TM::Escaped => TextMode::Escaped,
        }
    }
}

impl From<TextMode> for bindings::reader_options_ffi::TextMode {
    fn from(value: TextMode) -> Self {
        use bindings::reader_options_ffi::TextMode as TM;
        match value {
            TextMode::Plain => TM::Plain,
            TextMode::ECI => TM::ECI,
            TextMode::HRI => TM::HRI,
            TextMode::Hex => TM::Hex,
            TextMode::Escaped => TM::Escaped,
        }
    }
}

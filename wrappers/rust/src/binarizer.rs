use crate::bindings;

#[derive(Copy, Clone)]
pub enum Binarizer {
    LocalAverage,
    GlobalHistogram,
    FixedThreshold,
    BoolCast,
}

impl From<bindings::reader_options_ffi::Binarizer> for Binarizer {
    fn from(value: bindings::reader_options_ffi::Binarizer) -> Self {
        use bindings::reader_options_ffi::Binarizer as B;
        match value {
            B::LocalAverage => Binarizer::LocalAverage,
            B::GlobalHistogram => Binarizer::GlobalHistogram,
            B::FixedThreshold => Binarizer::FixedThreshold,
            B::BoolCast => Binarizer::BoolCast,
        }
    }
}

impl From<Binarizer> for bindings::reader_options_ffi::Binarizer {
    fn from(value: Binarizer) -> Self {
        use bindings::reader_options_ffi::Binarizer as B;
        match value {
            Binarizer::LocalAverage => B::LocalAverage,
            Binarizer::GlobalHistogram => B::GlobalHistogram,
            Binarizer::FixedThreshold => B::FixedThreshold,
            Binarizer::BoolCast => B::BoolCast,
        }
    }
}

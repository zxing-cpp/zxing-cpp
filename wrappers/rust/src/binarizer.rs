use crate::bindings;

#[derive(Copy, Clone)]
pub enum Binarizer {
    LocalAverage,
    GlobalHistogram,
    FixedThreshold,
    BoolCast,
}

impl From<bindings::wrapped_ffi::Binarizer> for Binarizer {
    fn from(value: bindings::wrapped_ffi::Binarizer) -> Self {
        use bindings::wrapped_ffi::Binarizer as B;
        match value {
            B::LocalAverage => Binarizer::LocalAverage,
            B::GlobalHistogram => Binarizer::GlobalHistogram,
            B::FixedThreshold => Binarizer::FixedThreshold,
            B::BoolCast => Binarizer::BoolCast,
        }
    }
}

impl From<Binarizer> for bindings::wrapped_ffi::Binarizer {
    fn from(value: Binarizer) -> Self {
        use bindings::wrapped_ffi::Binarizer as B;
        match value {
            Binarizer::LocalAverage => B::LocalAverage,
            Binarizer::GlobalHistogram => B::GlobalHistogram,
            Binarizer::FixedThreshold => B::FixedThreshold,
            Binarizer::BoolCast => B::BoolCast,
        }
    }
}

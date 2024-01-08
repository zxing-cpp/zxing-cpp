use crate::bindings;

/// The Binarizer enum
///
/// Specify which algorithm to use for grayscale to binary transformation.
/// The difference is how to get to a threshold value `T` which results in a bit
/// value `R = L <= T`
#[derive(Copy, Clone)]
pub enum Binarizer {
    /// `T` = Average of neighboring pixels for matrix and GlobalHistogram for linear (HybridBinarizer)
    LocalAverage,
    /// `T` = valley between the 2 largest peeks in the histogram (per line in linear case)
    GlobalHistogram,
    /// `T` = 127
    FixedThreshold,
    /// `T` = 0, fastest possible
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

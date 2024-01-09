#![allow(clippy::needless_lifetimes)]
#![allow(clippy::too_many_arguments)]
#![allow(clippy::upper_case_acronyms)]

pub(crate) mod base_ffi {
    use autocxx::prelude::*;
    use std::fmt::{Display, Formatter};
    include_cpp! {
        #include "ReaderOptionsExt.h"
        #include "ImageView.h"
        #include "Result.h"
        #include "Util.h"
        #include "ResultsExt.h"
        safety!(unsafe_ffi)
        generate!("ZXing::readBarcodes")
        generate!("ZXing::ImageView")
        generate!("ZXing::Result")
        generate!("ZXing::Results")
        generate!("ZXing::ToString")
        generate!("ZXing::contentTypeToString")
        generate!("ZXing::barcodeFormatToString")
        generate!("ZXing::errorToString")
        generate!("ZXing::byteArrayAsVec")
        generate!("ZXing::characterSetToString")
        generate!("ZXing::ReaderOptionsExt")
        generate!("ZXing::ResultsExt")
    }
    use crate::bindings;
    pub use ffi::ZXing::*;

    impl Display for CharacterSet {
        fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
            write!(f, "{}", characterSetToString(self.clone()))
        }
    }

    impl Display for ContentType {
        fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
            write!(f, "{}", contentTypeToString(self.clone()))
        }
    }
}

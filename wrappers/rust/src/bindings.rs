#![allow(clippy::needless_lifetimes)]
#![allow(clippy::too_many_arguments)]
#![allow(clippy::upper_case_acronyms)]

pub(crate) mod wrapped_ffi {
    use autocxx::include_cpp;
    include_cpp! {
        #include "ReaderOptionsExt.h"
        name!(reader_options_ext)
        safety!(unsafe_references_wrapped)
        generate!("ZXing::ReaderOptionsExt")
        extern_cpp_type!("ZXing::ReaderOptions", crate::bindings::wrapped_ffi::ReaderOptions)
        extern_cpp_type!("ZXing::BarcodeFormat", crate::bindings::base_ffi::BarcodeFormat)
    }

    include_cpp! {
        #include "ReaderOptions.h"
        name!(reader_options)
        safety!(unsafe_references_wrapped)
        generate!("ZXing::ReaderOptions")
        extern_cpp_type!("ZXing::BarcodeFormat", crate::bindings::base_ffi::BarcodeFormat)
    }

    pub use reader_options::ZXing::*;
    pub use reader_options_ext::ZXing::*;
}

pub(crate) mod base_ffi {
    use autocxx::prelude::*;
    include_cpp! {
        #include "ImageView.h"
        #include "ReadBarcode.h"
        #include "Result.h"
        #include "Flags.h"
        #include "Point.h"
        #include "BarcodeFormat.h"
        #include "Quadrilateral.h"
        #include "Util.h"
        #include "MultiFormatWriterExt.h"
        #include "MultiFormatWriter.h"
        #include "MatrixExt.h"
        safety!(unsafe_ffi)
        generate!("ZXing::ReadBarcode")
        generate!("ZXing::ImageView")
        generate!("ZXing::Result")
        generate!("ZXing::Results")
        generate!("ZXing::ToString")
        generate!("ZXing::ContentTypeToString")
        generate!("ZXing::BarcodeFormatToString")
        generate!("ZXing::ErrorToString")
        generate!("ZXing::ByteArrayAsVec")
        generate!("ZXing::CharacterSetToString")
        generate!("ZXing::MultiFormatWriterExt")
        generate!("ZXing::MultiFormatWriter")
        generate!("ZXing::MatrixExt")
        extern_cpp_type!("ZXing::ReaderOptions", crate::bindings::wrapped_ffi::ReaderOptions)
        extern_cpp_type!("ZXing::TextMode", crate::bindings::wrapped_ffi::TextMode)
        extern_cpp_type!("ZXing::CharacterSet", crate::bindings::wrapped_ffi::CharacterSet)
    }
    pub use ffi::ZXing::*;
}

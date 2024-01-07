use crate::bindings;
use std::fmt::{Display, Formatter};

#[derive(Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
#[repr(i32)]
pub enum BarcodeFormat {
    ///< Used as a return value if no valid barcode has been detected
    None = 0,
    ///< Aztec
    Aztec = 1,
    ///< Codabar
    Codabar = 2,
    ///< Code39
    Code39 = 4,
    ///< Code93
    Code93 = 8,
    ///< Code128
    Code128 = 16,
    ///< GS1 DataBar, formerly known as RSS 14
    DataBar = 32,
    ///< GS1 DataBar Expanded, formerly known as RSS EXPANDED
    DataBarExpanded = 64,
    ///< DataMatrix
    DataMatrix = 128,
    ///< EAN-8
    EAN8 = 256,
    ///< EAN-13
    EAN13 = 512,
    ///< ITF (Interleaved Two of Five)
    ITF = 1024,
    ///< MaxiCode
    MaxiCode = 2048,
    ///< PDF417
    PDF417 = 4096,
    ///< QR Code
    QRCode = 8192,
    ///< UPC-A
    UPCA = 16384,
    ///< UPC-E
    UPCE = 32768,
    ///< Micro QR Code
    MicroQRCode = 65536,
    ///< Rectangular Micro QR Code
    RMQRCode = 131072,
    ///< DX Film Edge Barcode
    DXFilmEdge = 262144,
    LinearCodes = 313214,
    MatrixCodes = 211073,
    Any = 524287,
}

impl From<bindings::base_ffi::BarcodeFormat> for BarcodeFormat {
    fn from(value: bindings::base_ffi::BarcodeFormat) -> Self {
        use bindings::base_ffi::BarcodeFormat as BF;
        match value {
            bindings::base_ffi::BarcodeFormat::None => BarcodeFormat::None,
            bindings::base_ffi::BarcodeFormat::Aztec => BarcodeFormat::Aztec,
            bindings::base_ffi::BarcodeFormat::Codabar => BarcodeFormat::Codabar,
            bindings::base_ffi::BarcodeFormat::Code39 => BarcodeFormat::Code39,
            bindings::base_ffi::BarcodeFormat::Code93 => BarcodeFormat::Code93,
            bindings::base_ffi::BarcodeFormat::Code128 => BarcodeFormat::Code128,
            bindings::base_ffi::BarcodeFormat::DataBar => BarcodeFormat::DataBar,
            bindings::base_ffi::BarcodeFormat::DataBarExpanded => BarcodeFormat::DataBarExpanded,
            bindings::base_ffi::BarcodeFormat::DataMatrix => BarcodeFormat::DataMatrix,
            bindings::base_ffi::BarcodeFormat::EAN8 => BarcodeFormat::EAN8,
            bindings::base_ffi::BarcodeFormat::EAN13 => BarcodeFormat::EAN13,
            bindings::base_ffi::BarcodeFormat::ITF => BarcodeFormat::ITF,
            bindings::base_ffi::BarcodeFormat::MaxiCode => BarcodeFormat::MaxiCode,
            bindings::base_ffi::BarcodeFormat::PDF417 => BarcodeFormat::PDF417,
            bindings::base_ffi::BarcodeFormat::QRCode => BarcodeFormat::QRCode,
            bindings::base_ffi::BarcodeFormat::UPCA => BarcodeFormat::UPCA,
            bindings::base_ffi::BarcodeFormat::UPCE => BarcodeFormat::UPCE,
            bindings::base_ffi::BarcodeFormat::MicroQRCode => BarcodeFormat::MicroQRCode,
            bindings::base_ffi::BarcodeFormat::RMQRCode => BarcodeFormat::RMQRCode,
            bindings::base_ffi::BarcodeFormat::DXFilmEdge => BarcodeFormat::DXFilmEdge,
            bindings::base_ffi::BarcodeFormat::LinearCodes => BarcodeFormat::LinearCodes,
            bindings::base_ffi::BarcodeFormat::MatrixCodes => BarcodeFormat::MatrixCodes,
            bindings::base_ffi::BarcodeFormat::Any => BarcodeFormat::Any,
        }
    }
}

impl From<BarcodeFormat> for bindings::base_ffi::BarcodeFormat {
    fn from(value: BarcodeFormat) -> Self {
        use bindings::base_ffi::BarcodeFormat as BF;
        match value {
            BarcodeFormat::None => bindings::base_ffi::BarcodeFormat::None,
            BarcodeFormat::Aztec => bindings::base_ffi::BarcodeFormat::Aztec,
            BarcodeFormat::Codabar => bindings::base_ffi::BarcodeFormat::Codabar,
            BarcodeFormat::Code39 => bindings::base_ffi::BarcodeFormat::Code39,
            BarcodeFormat::Code93 => bindings::base_ffi::BarcodeFormat::Code93,
            BarcodeFormat::Code128 => bindings::base_ffi::BarcodeFormat::Code128,
            BarcodeFormat::DataBar => bindings::base_ffi::BarcodeFormat::DataBar,
            BarcodeFormat::DataBarExpanded => bindings::base_ffi::BarcodeFormat::DataBarExpanded,
            BarcodeFormat::DataMatrix => bindings::base_ffi::BarcodeFormat::DataMatrix,
            BarcodeFormat::EAN8 => bindings::base_ffi::BarcodeFormat::EAN8,
            BarcodeFormat::EAN13 => bindings::base_ffi::BarcodeFormat::EAN13,
            BarcodeFormat::ITF => bindings::base_ffi::BarcodeFormat::ITF,
            BarcodeFormat::MaxiCode => bindings::base_ffi::BarcodeFormat::MaxiCode,
            BarcodeFormat::PDF417 => bindings::base_ffi::BarcodeFormat::PDF417,
            BarcodeFormat::QRCode => bindings::base_ffi::BarcodeFormat::QRCode,
            BarcodeFormat::UPCA => bindings::base_ffi::BarcodeFormat::UPCA,
            BarcodeFormat::UPCE => bindings::base_ffi::BarcodeFormat::UPCE,
            BarcodeFormat::MicroQRCode => bindings::base_ffi::BarcodeFormat::MicroQRCode,
            BarcodeFormat::RMQRCode => bindings::base_ffi::BarcodeFormat::RMQRCode,
            BarcodeFormat::DXFilmEdge => bindings::base_ffi::BarcodeFormat::DXFilmEdge,
            BarcodeFormat::LinearCodes => bindings::base_ffi::BarcodeFormat::LinearCodes,
            BarcodeFormat::MatrixCodes => bindings::base_ffi::BarcodeFormat::MatrixCodes,
            BarcodeFormat::Any => bindings::base_ffi::BarcodeFormat::Any,
        }
    }
}

impl Display for BarcodeFormat {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}",
            bindings::base_ffi::BarcodeFormatToString((*self).into())
        )
    }
}

use crate::bindings;
use flagset::flags;
use num_enum::FromPrimitive;
use std::fmt::{Display, Formatter};

flags! {
    #[repr(u32)]
    #[derive(FromPrimitive)]
    pub enum BarcodeFormat: u32 {
        #[num_enum(default)]
        None = 0,
        /// Aztec
        Aztec = 1 << 0,
        /// Codabar
        Codabar = 1 << 1,
        /// Code39
        Code39 = 1 << 2,
        /// Code93
        Code93 = 1 << 3,
        /// Code128
        Code128 = 1 << 4,
        /// GS1 DataBar, formerly known as RSS 14
        DataBar = 1 << 5,
        /// GS1 DataBar Expanded, formerly known as RSS EXPANDED
        DataBarExpanded = 1 << 6,
        /// DataMatrix
        DataMatrix = 1 << 7,
        /// EAN-8
        EAN8 = 1 << 8,
        /// EAN-13
        EAN13 = 1 << 9,
        /// ITF (Interleaved Two of Five)
        ITF = 1 << 10,
        /// MaxiCode
        MaxiCode = 1 << 11,
        /// PDF417
        PDF417 = 1 << 12,
        /// QR Code
        QRCode = 1 << 13,
        /// UPC-A
        UPCA = 1 << 14,
        /// UPC-E
        UPCE = 1 << 15,
        /// Micro QR Code
        MicroQRCode = 1 << 16,
        /// Rectangular Micro QR Code
        RMQRCode = 1 << 17,
        /// DX Film Edge Barcode
        DXFilmEdge = 1 << 18,

        LinearCodes = (BarcodeFormat::Codabar | BarcodeFormat::Code39 | BarcodeFormat::Code93 | BarcodeFormat::Code128 | BarcodeFormat::EAN8 | BarcodeFormat::EAN13 | BarcodeFormat::ITF | BarcodeFormat::DataBar | BarcodeFormat::DataBarExpanded | BarcodeFormat::DXFilmEdge | BarcodeFormat::UPCA | BarcodeFormat::UPCE).bits(),
        MatrixCodes = (BarcodeFormat::Aztec | BarcodeFormat::DataMatrix | BarcodeFormat::MaxiCode | BarcodeFormat::PDF417 | BarcodeFormat::QRCode | BarcodeFormat::MicroQRCode | BarcodeFormat::RMQRCode).bits(),
        Any = (BarcodeFormat::LinearCodes | BarcodeFormat::MatrixCodes).bits(),
    }
}

impl Display for BarcodeFormat {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}",
            bindings::ffi::barcode_format_to_string(*self as i32)
        )
    }
}

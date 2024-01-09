use clap::{Parser, ValueEnum};
use flagset::FlagSet;
use image::io::Reader;
use std::path::PathBuf;
use zxing_cpp2rs::{read_barcodes, BarcodeFormat, ImageView, ReaderOptions};

#[cfg(not(feature = "image"))]
use image::EncodableLayout;

#[cfg(not(feature = "image"))]
use zxing_cpp2rs::ImageFormat;

#[derive(Parser)]
struct Cli {
    filename: PathBuf,
    formats: Vec<BarcodeFormatArg>,
}

#[derive(ValueEnum, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub enum BarcodeFormatArg {
    ///< Used as a return value if no valid barcode has been detected
    None,
    ///< Aztec
    Aztec,
    ///< Codabar
    Codabar,
    ///< Code39
    Code39,
    ///< Code93
    Code93,
    ///< Code128
    Code128,
    ///< GS1 DataBar, formerly known as RSS 14
    DataBar,
    ///< GS1 DataBar Expanded, formerly known as RSS EXPANDED
    DataBarExpanded,
    ///< DataMatrix
    DataMatrix,
    ///< EAN-8
    EAN8,
    ///< EAN-13
    EAN13,
    ///< ITF (Interleaved Two of Five)
    ITF,
    ///< MaxiCode
    MaxiCode,
    ///< PDF417
    PDF417,
    ///< QR Code
    QRCode,
    ///< UPC-A
    UPCA,
    ///< UPC-E
    UPCE,
    ///< Micro QR Code
    MicroQRCode,
    ///< Rectangular Micro QR Code
    RMQRCode,
    ///< DX Film Edge Barcode
    DXFilmEdge,
    LinearCodes,
    MatrixCodes,
    Any,
}

impl From<BarcodeFormatArg> for BarcodeFormat {
    fn from(value: BarcodeFormatArg) -> Self {
        use BarcodeFormat as BF;
        match value {
            BarcodeFormatArg::None => BF::None,
            BarcodeFormatArg::Aztec => BF::Aztec,
            BarcodeFormatArg::Codabar => BF::Codabar,
            BarcodeFormatArg::Code39 => BF::Code39,
            BarcodeFormatArg::Code93 => BF::Code93,
            BarcodeFormatArg::Code128 => BF::Code128,
            BarcodeFormatArg::DataBar => BF::DataBar,
            BarcodeFormatArg::DataBarExpanded => BF::DataBarExpanded,
            BarcodeFormatArg::DataMatrix => BF::DataMatrix,
            BarcodeFormatArg::EAN8 => BF::EAN8,
            BarcodeFormatArg::EAN13 => BF::EAN13,
            BarcodeFormatArg::ITF => BF::ITF,
            BarcodeFormatArg::MaxiCode => BF::MaxiCode,
            BarcodeFormatArg::PDF417 => BF::PDF417,
            BarcodeFormatArg::QRCode => BF::QRCode,
            BarcodeFormatArg::UPCA => BF::UPCA,
            BarcodeFormatArg::UPCE => BF::UPCE,
            BarcodeFormatArg::MicroQRCode => BF::MicroQRCode,
            BarcodeFormatArg::RMQRCode => BF::RMQRCode,
            BarcodeFormatArg::DXFilmEdge => BF::DXFilmEdge,
            BarcodeFormatArg::LinearCodes => BF::LinearCodes,
            BarcodeFormatArg::MatrixCodes => BF::MatrixCodes,
            BarcodeFormatArg::Any => BF::Any,
        }
    }
}

fn main() -> anyhow::Result<()> {
    // Parse arguments
    let cli = Cli::parse();

    // Load image from file and convert to GrayImage
    let img = Reader::open(cli.filename)?.decode()?.to_luma8();

    #[cfg(not(feature = "image"))]
    // Get image width
    let width = img.width();
    #[cfg(not(feature = "image"))]
    // Get image height
    let height = img.height();

    #[cfg(not(feature = "image"))]
    let image = ImageView::new(img.as_bytes(), width, height, ImageFormat::Lum, 0, 0);

    #[cfg(feature = "image")]
    let image = ImageView::from_dynamic_image(&img);

    // Create format flags based on cli arguments
    let formats = cli
        .formats
        .into_iter()
        .fold(FlagSet::<BarcodeFormat>::new_truncated(0), |acc, format| {
            acc | BarcodeFormat::from(format)
        });

    // Create options and set formats
    let options = ReaderOptions::default().set_formats(formats);

    // Read barcodes from the provided image
    let results = read_barcodes(&image, &options);
    let last = results.len() - 1;
    for (i, result) in results.into_iter().enumerate() {
        if !result.is_valid() {
            println!("No barcode found");
        } else {
            println!("Text: {}", result.text());
            println!("Format: {}", result.format());
            println!("Content: {}", result.content_type());
            println!("Identifier: {}", result.symbology_identifier());
            println!("EC Level: {}", result.ec_level());
            println!("Error: {}", result.error_message());
            println!("Rotation: {}", result.orientation());
        }
        if i != last {
            println!();
        }
    }

    Ok(())
}

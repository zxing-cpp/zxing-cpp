use image::{ImageBuffer, Luma};
use zxing_cpp2rs::{BarcodeFormat, MultiFormatWriter};

fn main() -> anyhow::Result<()> {
    let contents = "1234567890";
    let mut writer = MultiFormatWriter::new(BarcodeFormat::Codabar);
    writer.margin(100);

    let matrix = writer.encode_to_matrix(contents, 100, 100);

    let data = matrix.data();

    let image: ImageBuffer<Luma<u8>, &[u8]> =
        ImageBuffer::from_raw(matrix.width(), matrix.height(), data)
            .ok_or(anyhow::Error::msg("Failed to create image buffer"))?;

    image.save("barcode.png")?;

    Ok(())
}

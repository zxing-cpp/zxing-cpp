use crate::bindings;

/// Enum representing the format of an image's pixels
pub enum ImageFormat {
    None = 0,
    Lum = 0x01000000,
    RGB = 0x03000102,
    BGR = 0x03020100,
    RGBX = 0x04000102,
    XRGB = 0x04010203,
    BGRX = 0x04020100,
    XBGR = 0x04030201,
}

impl From<bindings::base_ffi::ImageFormat> for ImageFormat {
    fn from(value: bindings::base_ffi::ImageFormat) -> Self {
        use bindings::base_ffi::ImageFormat as IF;
        match value {
            IF::None => ImageFormat::None,
            IF::Lum => ImageFormat::Lum,
            IF::RGB => ImageFormat::RGB,
            IF::BGR => ImageFormat::BGR,
            IF::RGBX => ImageFormat::RGBX,
            IF::XRGB => ImageFormat::XRGB,
            IF::BGRX => ImageFormat::BGRX,
            IF::XBGR => ImageFormat::XBGR,
        }
    }
}

impl From<ImageFormat> for bindings::base_ffi::ImageFormat {
    fn from(value: ImageFormat) -> Self {
        use bindings::base_ffi::ImageFormat as IF;
        match value {
            ImageFormat::None => IF::None,
            ImageFormat::Lum => IF::Lum,
            ImageFormat::RGB => IF::RGB,
            ImageFormat::BGR => IF::BGR,
            ImageFormat::RGBX => IF::RGBX,
            ImageFormat::XRGB => IF::XRGB,
            ImageFormat::BGRX => IF::BGRX,
            ImageFormat::XBGR => IF::XBGR,
        }
    }
}

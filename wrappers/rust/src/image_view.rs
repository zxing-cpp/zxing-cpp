use crate::bindings;
use cxx::UniquePtr;
use std::marker::PhantomData;

use crate::bindings::ffi::ImageFormat;
#[cfg(feature = "image")]
use image::DynamicImage;

/// Struct that stores a reference to image data plus layout and formation information.
pub struct ImageView<'a> {
    data: PhantomData<&'a [u8]>,
    pub(crate) image: UniquePtr<bindings::ffi::ImageView>,
}

impl<'a> ImageView<'a> {
    /// Creates a new [ImageView]
    ///
    /// * `data` - reference to image buffer
    /// * `width` - image width in pixels
    /// * `height` - image height in pixels
    /// * `format` - image/pixel format
    /// * `row_stride` - row stride in bytes. If 0, the default is `width` * `pix_stride`
    /// * `pix_stride` - pixel stride in bytes. If 0, the default is calculated from format
    pub fn new(
        data: &'a [u8],
        width: u32,
        height: u32,
        format: ImageFormat,
        row_stride: u32,
        pix_stride: u32,
    ) -> Self {
        Self {
            image: unsafe {
                bindings::ffi::new_image_view(
                    data.as_ptr(),
                    width as i32,
                    height as i32,
                    format,
                    row_stride as i32,
                    pix_stride as i32,
                )
            },
            data: PhantomData,
        }
    }

    #[cfg(feature = "image")]
    /// Creates an [ImageView] from a [DynamicImage]
    ///
    /// Returns `None` when the underlying buffer does not match a compatible [ImageFormat]
    pub fn from_dynamic_image(image: &'a DynamicImage) -> Option<Self> {
        let format = match image {
            DynamicImage::ImageLuma8(_) => Some(ImageFormat::Lum),
            DynamicImage::ImageRgb8(_) => Some(ImageFormat::RGB),
            DynamicImage::ImageRgba8(_) => Some(ImageFormat::RGBX),
            _ => None,
        };

        Some(Self {
            data: PhantomData,
            image: unsafe {
                bindings::ffi::new_image_view(
                    image.as_bytes().as_ptr(),
                    image.width() as i32,
                    image.height() as i32,
                    format?,
                    0,
                    0,
                )
            },
        })
    }
}

use crate::{bindings, ImageFormat};
use autocxx::{c_int, WithinUniquePtr};
use cxx::UniquePtr;
use std::io::Cursor;
use std::marker::PhantomData;

#[cfg(feature = "image")]
use image::{DynamicImage, EncodableLayout, GrayImage};

/// Struct that stores a reference to image data plus layout and formation information.
pub struct ImageView<'a> {
    _data: PhantomData<&'a [u8]>,
    pub(crate) image: UniquePtr<bindings::base_ffi::ImageView>,
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
                bindings::base_ffi::ImageView::new(
                    data.as_ptr(),
                    c_int(width as i32),
                    c_int(height as i32),
                    format,
                    c_int(row_stride as i32),
                    c_int(pix_stride as i32),
                )
            }
            .within_unique_ptr(),
            _data: PhantomData,
        }
    }

    #[cfg(feature = "image")]
    /// Creates an [ImageView] from a [GrayImage]
    pub fn from_dynamic_image(image: &'a GrayImage) -> Self {
        Self {
            _data: PhantomData,
            image: unsafe {
                bindings::base_ffi::ImageView::new(
                    image.as_bytes().as_ptr(),
                    c_int(image.width() as i32),
                    c_int(image.height() as i32),
                    ImageFormat::Lum,
                    c_int(0),
                    c_int(0),
                )
            }
            .within_unique_ptr(),
        }
    }
}

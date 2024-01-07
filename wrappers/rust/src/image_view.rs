use crate::bindings;
use crate::image_format::ImageFormat;
use autocxx::{c_int, WithinUniquePtr};
use cxx::UniquePtr;
use std::io::Cursor;

pub struct ImageView<'a> {
    _data: &'a [u8],
    pub(crate) image: UniquePtr<bindings::base_ffi::ImageView>,
}

impl<'a> ImageView<'a> {
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
                    format.into(),
                    c_int(row_stride as i32),
                    c_int(pix_stride as i32),
                )
            }
            .within_unique_ptr(),
            _data: data,
        }
    }
}

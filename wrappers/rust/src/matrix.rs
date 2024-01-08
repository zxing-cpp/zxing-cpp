use crate::bindings;
use cxx::UniquePtr;

pub struct Matrix {
    matrix: UniquePtr<bindings::base_ffi::MatrixExt>,
}

impl From<UniquePtr<bindings::base_ffi::MatrixExt>> for Matrix {
    fn from(value: UniquePtr<bindings::base_ffi::MatrixExt>) -> Self {
        Self { matrix: value }
    }
}

impl Matrix {
    pub fn data(&self) -> &[u8] {
        let size = self.matrix.size().0 as usize;
        unsafe { std::slice::from_raw_parts(self.matrix.data(), size) }
    }

    pub fn height(&self) -> u32 {
        self.matrix.height().0 as u32
    }

    pub fn width(&self) -> u32 {
        self.matrix.width().0 as u32
    }
}

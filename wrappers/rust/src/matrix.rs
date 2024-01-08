use crate::bindings;
use cxx::UniquePtr;

/// Struct holding encoded image data as a matrix of bytes
pub struct Matrix {
    matrix: UniquePtr<bindings::base_ffi::MatrixExt>,
}

impl From<UniquePtr<bindings::base_ffi::MatrixExt>> for Matrix {
    fn from(value: UniquePtr<bindings::base_ffi::MatrixExt>) -> Self {
        Self { matrix: value }
    }
}

impl Matrix {
    /// Data contained within the matrix as a slice of bytes
    pub fn data(&self) -> &[u8] {
        let size = self.matrix.size().0 as usize;
        unsafe { std::slice::from_raw_parts(self.matrix.data(), size) }
    }

    /// Height of the matrix
    pub fn height(&self) -> u32 {
        self.matrix.height().0 as u32
    }

    /// Width of the matrix
    pub fn width(&self) -> u32 {
        self.matrix.width().0 as u32
    }
}

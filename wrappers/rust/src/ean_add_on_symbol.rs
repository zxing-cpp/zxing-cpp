use crate::bindings;

/// Enum representing what to do with the EAN-2/5 Add-On symbol
#[derive(Copy, Clone)]
pub enum EanAddOnSymbol {
    /// Ignore any Add-On symbol during read/scan
    Ignore,
    /// Read EAN-2/EAN-5 Add-On symbol if found
    Read,
    /// Require EAN-2/EAN-5 Add-On symbol to be present
    Require,
}

impl From<bindings::wrapped_ffi::EanAddOnSymbol> for EanAddOnSymbol {
    fn from(value: bindings::wrapped_ffi::EanAddOnSymbol) -> Self {
        use bindings::wrapped_ffi::EanAddOnSymbol as ES;
        match value {
            ES::Ignore => EanAddOnSymbol::Ignore,
            ES::Read => EanAddOnSymbol::Read,
            ES::Require => EanAddOnSymbol::Require,
        }
    }
}

impl From<EanAddOnSymbol> for bindings::wrapped_ffi::EanAddOnSymbol {
    fn from(value: EanAddOnSymbol) -> Self {
        use bindings::wrapped_ffi::EanAddOnSymbol as ES;
        match value {
            EanAddOnSymbol::Ignore => ES::Ignore,
            EanAddOnSymbol::Read => ES::Read,
            EanAddOnSymbol::Require => ES::Require,
        }
    }
}

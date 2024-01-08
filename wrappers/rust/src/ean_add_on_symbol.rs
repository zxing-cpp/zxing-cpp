use crate::bindings;

#[derive(Copy, Clone)]
pub enum EanAddOnSymbol {
    Ignore,
    Read,
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

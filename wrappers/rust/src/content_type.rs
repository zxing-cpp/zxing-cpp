use crate::bindings;
use std::fmt::{Display, Formatter};

#[derive(Copy, Clone)]
pub enum ContentType {
    Text,
    Binary,
    Mixed,
    GS1,
    ISO15434,
    UnknownECI,
}

impl From<bindings::base_ffi::ContentType> for ContentType {
    fn from(value: bindings::base_ffi::ContentType) -> Self {
        use bindings::base_ffi::ContentType as CT;
        match value {
            CT::Text => ContentType::Text,
            CT::Binary => ContentType::Binary,
            CT::Mixed => ContentType::Mixed,
            CT::GS1 => ContentType::GS1,
            CT::ISO15434 => ContentType::ISO15434,
            CT::UnknownECI => ContentType::UnknownECI,
        }
    }
}

impl From<ContentType> for bindings::base_ffi::ContentType {
    fn from(value: ContentType) -> Self {
        use bindings::base_ffi::ContentType as CT;
        match value {
            ContentType::Text => CT::Text,
            ContentType::Binary => CT::Binary,
            ContentType::Mixed => CT::Mixed,
            ContentType::GS1 => CT::GS1,
            ContentType::ISO15434 => CT::ISO15434,
            ContentType::UnknownECI => CT::UnknownECI,
        }
    }
}

impl Display for ContentType {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}",
            bindings::base_ffi::ContentTypeToString((*self).into())
        )
    }
}

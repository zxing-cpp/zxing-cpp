#![allow(clippy::upper_case_acronyms)]

use crate::bindings;
use autocxx::WithinUniquePtrTrivial;
use std::fmt::{Display, Formatter};

#[derive(Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
#[repr(u8)]
pub enum CharacterSet {
    Unknown,
    ASCII,
    ISO8859_1,
    ISO8859_2,
    ISO8859_3,
    ISO8859_4,
    ISO8859_5,
    ISO8859_6,
    ISO8859_7,
    ISO8859_8,
    ISO8859_9,
    ISO8859_10,
    ISO8859_11,
    ISO8859_13,
    ISO8859_14,
    ISO8859_15,
    ISO8859_16,
    Cp437,
    Cp1250,
    Cp1251,
    Cp1252,
    Cp1256,

    ShiftJIS,
    Big5,
    GB2312,
    GB18030,
    EUCJP,
    EUCKR,
    UTF16BE,
    UTF8,
    UTF16LE,
    UTF32BE,
    UTF32LE,

    BINARY,

    CharsetCount,
}

impl From<bindings::wrapped_ffi::CharacterSet> for CharacterSet {
    fn from(value: bindings::wrapped_ffi::CharacterSet) -> Self {
        use bindings::wrapped_ffi::CharacterSet as CS;
        match value {
            CS::Unknown => CharacterSet::Unknown,
            CS::ASCII => CharacterSet::ASCII,
            CS::ISO8859_1 => CharacterSet::ISO8859_1,
            CS::ISO8859_2 => CharacterSet::ISO8859_2,
            CS::ISO8859_3 => CharacterSet::ISO8859_3,
            CS::ISO8859_4 => CharacterSet::ISO8859_4,
            CS::ISO8859_5 => CharacterSet::ISO8859_5,
            CS::ISO8859_6 => CharacterSet::ISO8859_6,
            CS::ISO8859_7 => CharacterSet::ISO8859_7,
            CS::ISO8859_8 => CharacterSet::ISO8859_8,
            CS::ISO8859_9 => CharacterSet::ISO8859_9,
            CS::ISO8859_10 => CharacterSet::ISO8859_10,
            CS::ISO8859_11 => CharacterSet::ISO8859_11,
            CS::ISO8859_13 => CharacterSet::ISO8859_13,
            CS::ISO8859_14 => CharacterSet::ISO8859_14,
            CS::ISO8859_15 => CharacterSet::ISO8859_15,
            CS::ISO8859_16 => CharacterSet::ISO8859_16,
            CS::Cp437 => CharacterSet::Cp437,
            CS::Cp1250 => CharacterSet::Cp1250,
            CS::Cp1251 => CharacterSet::Cp1251,
            CS::Cp1252 => CharacterSet::Cp1252,
            CS::Cp1256 => CharacterSet::Cp1256,
            CS::Shift_JIS => CharacterSet::ShiftJIS,
            CS::Big5 => CharacterSet::Big5,
            CS::GB2312 => CharacterSet::GB2312,
            CS::GB18030 => CharacterSet::GB18030,
            CS::EUC_JP => CharacterSet::EUCJP,
            CS::EUC_KR => CharacterSet::EUCKR,
            CS::UTF16BE => CharacterSet::UTF16BE,
            CS::UTF8 => CharacterSet::UTF8,
            CS::UTF16LE => CharacterSet::UTF16LE,
            CS::UTF32BE => CharacterSet::UTF32BE,
            CS::UTF32LE => CharacterSet::UTF32LE,
            CS::BINARY => CharacterSet::BINARY,
            CS::CharsetCount => CharacterSet::CharsetCount,
        }
    }
}

impl From<CharacterSet> for bindings::wrapped_ffi::CharacterSet {
    fn from(value: CharacterSet) -> Self {
        use bindings::wrapped_ffi::CharacterSet as CS;
        match value {
            CharacterSet::Unknown => CS::Unknown,
            CharacterSet::ASCII => CS::ASCII,
            CharacterSet::ISO8859_1 => CS::ISO8859_1,
            CharacterSet::ISO8859_2 => CS::ISO8859_2,
            CharacterSet::ISO8859_3 => CS::ISO8859_3,
            CharacterSet::ISO8859_4 => CS::ISO8859_4,
            CharacterSet::ISO8859_5 => CS::ISO8859_5,
            CharacterSet::ISO8859_6 => CS::ISO8859_6,
            CharacterSet::ISO8859_7 => CS::ISO8859_7,
            CharacterSet::ISO8859_8 => CS::ISO8859_8,
            CharacterSet::ISO8859_9 => CS::ISO8859_9,
            CharacterSet::ISO8859_10 => CS::ISO8859_10,
            CharacterSet::ISO8859_11 => CS::ISO8859_11,
            CharacterSet::ISO8859_13 => CS::ISO8859_13,
            CharacterSet::ISO8859_14 => CS::ISO8859_14,
            CharacterSet::ISO8859_15 => CS::ISO8859_15,
            CharacterSet::ISO8859_16 => CS::ISO8859_16,
            CharacterSet::Cp437 => CS::Cp437,
            CharacterSet::Cp1250 => CS::Cp1250,
            CharacterSet::Cp1251 => CS::Cp1251,
            CharacterSet::Cp1252 => CS::Cp1252,
            CharacterSet::Cp1256 => CS::Cp1256,
            CharacterSet::ShiftJIS => CS::Shift_JIS,
            CharacterSet::Big5 => CS::Big5,
            CharacterSet::GB2312 => CS::GB2312,
            CharacterSet::GB18030 => CS::GB18030,
            CharacterSet::EUCJP => CS::EUC_JP,
            CharacterSet::EUCKR => CS::EUC_KR,
            CharacterSet::UTF16BE => CS::UTF16BE,
            CharacterSet::UTF8 => CS::UTF8,
            CharacterSet::UTF16LE => CS::UTF16LE,
            CharacterSet::UTF32BE => CS::UTF32BE,
            CharacterSet::UTF32LE => CS::UTF32LE,
            CharacterSet::BINARY => CS::BINARY,
            CharacterSet::CharsetCount => CS::CharsetCount,
        }
    }
}

impl Display for CharacterSet {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}",
            bindings::base_ffi::CharacterSetToString(
                bindings::base_ffi::CharacterSet::from(*self).within_unique_ptr()
            )
        )
    }
}

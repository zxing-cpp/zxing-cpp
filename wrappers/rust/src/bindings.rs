use std::fmt::{Display, Formatter};

#[cxx::bridge(namespace = "ZXing")]
pub(crate) mod ffi {

    #[repr(u32)]
    enum ContentType {
        Text,
        Binary,
        Mixed,
        GS1,
        ISO15434,
        UnknownECI,
    }

    #[repr(u8)]
    enum CharacterSet {
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

        Shift_JIS,
        Big5,
        GB2312,
        GB18030,
        EUC_JP,
        EUC_KR,
        UTF16BE,
        UTF8,
        UTF16LE,
        UTF32BE,
        UTF32LE,

        BINARY,

        CharsetCount,
    }

    #[repr(u32)]
    enum ImageFormat {
        None = 0,
        Lum = 0x01000000,
        RGB = 0x03000102,
        BGR = 0x03020100,
        RGBX = 0x04000102,
        XRGB = 0x04010203,
        BGRX = 0x04020100,
        XBGR = 0x04030201,
    }

    #[repr(u8)]
    enum Binarizer {
        LocalAverage,
        GlobalHistogram,
        FixedThreshold,
        BoolCast,
    }

    #[repr(u8)]
    enum EanAddOnSymbol {
        Ignore,
        Read,
        Require,
    }

    #[repr(u8)]
    enum TextMode {
        Plain,
        ECI,
        HRI,
        Hex,
        Escaped,
    }

    unsafe extern "C++" {
        include!("Util.h");

        type CharacterSet;
        type ContentType;
        type Result;
        type ImageFormat;
        type ImageView;
        type Binarizer;
        type EanAddOnSymbol;
        type TextMode;
        type ReaderOptionsExt;

        type BarcodeFormat;

        type ByteArray;

        type Error;

        #[rust_name = "new_reader_options"]
        fn newReaderOptions() -> UniquePtr<ReaderOptionsExt>;

        #[rust_name = "set_formats"]
        fn setFormats(self: Pin<&mut ReaderOptionsExt>, formats: i32)
            -> Pin<&mut ReaderOptionsExt>;

        #[rust_name = "try_harder"]
        fn tryHarder(
            self: Pin<&mut ReaderOptionsExt>,
            try_harder: bool,
        ) -> Pin<&mut ReaderOptionsExt>;

        #[rust_name = "try_rotate"]
        fn tryRotate(
            self: Pin<&mut ReaderOptionsExt>,
            try_rotate: bool,
        ) -> Pin<&mut ReaderOptionsExt>;

        #[rust_name = "try_invert"]
        fn tryInvert(
            self: Pin<&mut ReaderOptionsExt>,
            try_invert: bool,
        ) -> Pin<&mut ReaderOptionsExt>;

        #[rust_name = "try_downscale"]
        fn tryDownscale(
            self: Pin<&mut ReaderOptionsExt>,
            try_downscale: bool,
        ) -> Pin<&mut ReaderOptionsExt>;

        fn pure(self: Pin<&mut ReaderOptionsExt>, pure: bool) -> Pin<&mut ReaderOptionsExt>;

        #[rust_name = "return_errors"]
        fn returnErrors(
            self: Pin<&mut ReaderOptionsExt>,
            return_errors: bool,
        ) -> Pin<&mut ReaderOptionsExt>;

        fn binarizer(
            self: Pin<&mut ReaderOptionsExt>,
            binarizer: Binarizer,
        ) -> Pin<&mut ReaderOptionsExt>;

        #[rust_name = "ean_add_on_symbol"]
        fn eanAddOnSymbol(
            self: Pin<&mut ReaderOptionsExt>,
            ean_add_on_symbol: EanAddOnSymbol,
        ) -> Pin<&mut ReaderOptionsExt>;

        #[rust_name = "text_mode"]
        fn textMode(
            self: Pin<&mut ReaderOptionsExt>,
            text_mode: TextMode,
        ) -> Pin<&mut ReaderOptionsExt>;

        #[rust_name = "new_image_view"]
        unsafe fn newImageView(
            data: *const u8,
            width: i32,
            height: i32,
            format: ImageFormat,
            rowStride: i32,
            pixStride: i32,
        ) -> UniquePtr<ImageView>;

        #[rust_name = "character_set_to_string"]
        fn characterSetToString(cs: CharacterSet) -> String;

        #[rust_name = "barcode_format_to_string"]
        fn barcodeFormatToString(f: i32) -> String;

        #[rust_name = "content_type_to_string"]
        fn contentTypeToString(ct: ContentType) -> String;

        #[rust_name = "is_valid"]
        fn isValid(self: &Result) -> bool;

        #[rust_name = "format_of_result"]
        fn formatOfResult(res: &Result) -> i32;

        fn bytes(self: &Result) -> &ByteArray;

        #[rust_name = "byte_array_as_vec"]
        fn byteArrayAsVec(ba: &ByteArray) -> &CxxVector<u8>;

        fn error(self: &Result) -> &Error;

        #[rust_name = "error_to_string"]
        fn errorToString(err: &Error) -> String;

        #[rust_name = "content_type"]
        fn contentType(self: &Result) -> ContentType;

        #[rust_name = "text_of_result"]
        fn textOfResult(res: &Result) -> String;

        #[rust_name = "ec_level_of_result"]
        fn ecLevelOfResult(res: &Result) -> String;

        #[rust_name = "symbology_identifier_of_result"]
        fn symbologyIdentifierOfResult(res: &Result) -> String;

        fn orientation(self: &Result) -> i32;

        #[rust_name = "is_mirrored"]
        fn isMirrored(self: &Result) -> bool;

        #[rust_name = "is_inverted"]
        fn isInverted(self: &Result) -> bool;

        #[rust_name = "read_barcodes"]
        fn readBarcodes(
            image: &ImageView,
            options: &ReaderOptionsExt,
        ) -> Result<UniquePtr<CxxVector<Result>>>;
    }
}

impl Display for ffi::CharacterSet {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", ffi::character_set_to_string(*self))
    }
}

impl Display for ffi::ContentType {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", ffi::content_type_to_string(*self))
    }
}

import enum
from collections.abc import Buffer
from typing import Any, Iterable, Iterator, TypeAlias, overload
from warnings import deprecated

# MARK: - Enums

class BarcodeFormat(enum.Enum):
    """
    Enumeration of zxing supported barcode formats
    """

    All: BarcodeFormat
    AllReadable: BarcodeFormat
    AllCreatable: BarcodeFormat
    AllLinear: BarcodeFormat
    AllMatrix: BarcodeFormat
    AllGS1: BarcodeFormat
    AllRetail: BarcodeFormat
    AllIndustrial: BarcodeFormat
    Codabar: BarcodeFormat
    Code39: BarcodeFormat
    Code39Std: BarcodeFormat
    Code39Ext: BarcodeFormat
    Code32: BarcodeFormat
    PZN: BarcodeFormat
    Code93: BarcodeFormat
    Code128: BarcodeFormat
    ITF: BarcodeFormat
    ITF14: BarcodeFormat
    DataBar: BarcodeFormat
    DataBarOmni: BarcodeFormat
    DataBarStk: BarcodeFormat
    DataBarStkOmni: BarcodeFormat
    DataBarLtd: BarcodeFormat
    DataBarExp: BarcodeFormat
    DataBarExpStk: BarcodeFormat
    EANUPC: BarcodeFormat
    EAN13: BarcodeFormat
    EAN8: BarcodeFormat
    EAN5: BarcodeFormat
    EAN2: BarcodeFormat
    ISBN: BarcodeFormat
    UPCA: BarcodeFormat
    UPCE: BarcodeFormat
    OtherBarcode: BarcodeFormat
    DXFilmEdge: BarcodeFormat
    PDF417: BarcodeFormat
    CompactPDF417: BarcodeFormat
    MicroPDF417: BarcodeFormat
    Aztec: BarcodeFormat
    AztecCode: BarcodeFormat
    AztecRune: BarcodeFormat
    QRCode: BarcodeFormat
    QRCodeModel1: BarcodeFormat
    QRCodeModel2: BarcodeFormat
    MicroQRCode: BarcodeFormat
    RMQRCode: BarcodeFormat
    DataMatrix: BarcodeFormat
    MaxiCode: BarcodeFormat
    # Backward compatibility alias:
    NONE: BarcodeFormat
    DataBarExpanded: BarcodeFormat
    DataBarLimited: BarcodeFormat
    LinearCodes: BarcodeFormat
    MatrixCodes: BarcodeFormat

    def __init__(self) -> None:
        self.symbology: BarcodeFormat

    @deprecated("operator | is deprecated, pass array or tuple instead.")
    def __or__(self, other: BarcodeFormat) -> BarcodeFormats: ...
    def __str__(self) -> str: ...

class BarcodeFormats:
    def __repr__(self) -> str: ...
    def __eq__(self, other: object) -> bool: ...
    @deprecated("operator | is deprecated, pass array or tuple instead.")
    def __or__(self, other: BarcodeFormat) -> BarcodeFormats: ...
    def __len__(self) -> int: ...
    def __iter__(self) -> Iterator[BarcodeFormat]: ...  # noqa: S2876
    def __getitem__(self, index: int) -> BarcodeFormat: ...
    @overload
    def __init__(self) -> None: ...
    @overload
    def __init__(self, values: BarcodeFormatsLike) -> None: ...

BarcodeFormatsLike: TypeAlias = BarcodeFormats | BarcodeFormat | Iterable[BarcodeFormat]

class Binarizer(enum.Enum):
    """
    Enumeration of binarizers used before decoding images
    """

    BoolCast: Binarizer
    FixedThreshold: Binarizer
    GlobalHistogram: Binarizer
    LocalAverage: Binarizer

class EanAddOnSymbol(enum.Enum):
    """
    Enumeration of options for EAN-2/5 add-on symbols check
    """

    Ignore: EanAddOnSymbol
    """Ignore any Add-On symbol during read/scan"""

    Read: EanAddOnSymbol
    """Read EAN-2/EAN-5 Add-On symbol if found"""

    Require: EanAddOnSymbol
    """Require EAN-2/EAN-5 Add-On symbol to be present"""

class ContentType(enum.Enum):
    """
    Enumeration of content types
    """

    Text: ContentType
    Binary: ContentType
    Mixed: ContentType
    GS1: ContentType
    ISO15434: ContentType
    UnknownECI: ContentType

class TextMode(enum.Enum):

    Plain: TextMode
    """bytes() transcoded to unicode based on ECI info or guessed charset (the default mode prior to 2.0)"""

    ECI: TextMode
    """standard content following the ECI protocol with every character set ECI segment transcoded to unicode"""

    HRI: TextMode
    """Human Readable Interpretation (dependent on the ContentType)"""

    Escaped: TextMode
    """Use the EscapeNonGraphical() function (e.g. ASCII 29 will be transcoded to '<GS>'"""

    Hex: TextMode
    """bytes() transcoded to ASCII string of HEX values"""

    HexECI: TextMode
    """bytesECI() transcoded to ASCII string of HEX values"""

class ImageFormat(enum.Enum):
    """
    Enumeration of image formats supported by read_barcodes
    """

    Lum: ImageFormat
    LumA: ImageFormat
    RGB: ImageFormat
    BGR: ImageFormat
    RGBA: ImageFormat
    ARGB: ImageFormat
    BGRA: ImageFormat
    ABGR: ImageFormat

# MARK: - Classes

class PointI:
    """
    Represents the coordinates of a point in an image
    """

    @property
    def x(self) -> int:
        """horizontal coordinate of the point"""

    @property
    def y(self) -> int:
        """vertical coordinate of the point"""

class Position:
    """
    The position of a decoded symbol
    """

    @property
    def top_left(self) -> PointI:
        """coordinate of the symbol's top-left corner"""

    @property
    def top_right(self) -> PointI:
        """coordinate of the symbol's top-right corner"""

    @property
    def bottom_left(self) -> PointI:
        """coordinate of the symbol's bottom-left corner"""

    @property
    def bottom_right(self) -> PointI:
        """coordinate of the symbol's bottom-right corner"""

class ErrorType(enum.Enum):

    Format: ErrorType
    """Data format error"""

    Checksum: ErrorType
    """Checksum error"""

    Unsupported: ErrorType
    """Unsupported content error"""

class Error:
    """
    Barcode reading error
    """

    @property
    def type(self) -> ErrorType:
        """Error type"""

    @property
    def message(self) -> str:
        """Error message"""

    def __str__(self) -> str: ...

class Barcode:
    """
    The Barcode class
    """

    @property
    def valid(self) -> bool:
        """whether or not barcode is valid (i.e. a symbol was found and decoded)"""

    @property
    def text(self) -> str:
        """text of the decoded symbol (see also TextMode parameter)"""

    @property
    def bytes(self) -> bytes:
        """uninterpreted bytes of the decoded symbol"""

    @property
    def format(self) -> BarcodeFormat:
        """decoded symbol format"""

    @property
    def symbology(self) -> BarcodeFormat:
        """decoded symbol symbology"""

    @property
    def symbology_identifier(self) -> str:
        """decoded symbology identifier"""

    @property
    def ec_level(self) -> str:
        """error correction level of the symbol (empty string if not applicable)"""

    @property
    def content_type(self) -> ContentType:
        """content type of symbol"""

    @property
    def position(self) -> Position:
        """position of the decoded symbol"""

    @property
    def orientation(self) -> int:
        """orientation (in degree) of the decoded symbol"""

    @property
    def error(self) -> Error | None:
        """Error code or None"""

    @property
    def extra(self) -> dict[Any, Any]:
        """Symbology specific extra information as a Python dictionary (might be empty)"""

    def to_image(
        self, scale: int = 1, add_hrt: bool = False, add_quiet_zones: bool = True
    ) -> Image: ...
    def to_svg(
        self, scale: int = 1, add_hrt: bool = False, add_quiet_zones: bool = True
    ) -> Image: ...

@deprecated("Use Barcode instead.")
class Result(Barcode): ...

# MARK: - Functions

def barcode_format_from_str(str: str) -> BarcodeFormat:
    """
    Convert string to BarcodeFormat\n\n
    :type str: str\n
    :param str: string representing barcode format\n
    :return: corresponding barcode format\n
    :rtype: zxingcpp.BarcodeFormat
    """

def barcode_formats_from_str(str: str) -> BarcodeFormats:
    """
    Convert string to BarcodeFormats\n\n
    :type str: str\n
    :param str: string representing a list of barcodes formats\n
    :return: corresponding barcode formats\n
    :rtype: zxingcpp.BarcodeFormats
    """

def barcode_formats_list(filter: BarcodeFormatsLike) -> list[BarcodeFormat]:
    """
    Returns a list of available/supported barcode formats, optionally filtered by the provided format(s).\n\n
    :type filter: zxingcpp.BarcodeFormats\n
    :param filter: the BarcodeFormat(s) to filter by\n
    :return: list of available/supported barcode formats (optionally filtered)\n
    :rtype: list[zxingcpp.BarcodeFormat]
    """

def read_barcode(
    image: object,
    formats: BarcodeFormatsLike = BarcodeFormats(),
    try_rotate: bool = True,
    try_downscale: bool = True,
    try_invert: bool = True,
    text_mode: TextMode = TextMode.HRI,
    binarizer: Binarizer = Binarizer.LocalAverage,
    is_pure: bool = False,
    ean_add_on_symbol: EanAddOnSymbol = EanAddOnSymbol.Ignore,
    return_errors: bool = False,
) -> Barcode | None:
    """
    Read (decode) a barcode from a numpy BGR or grayscale image array or from a PIL image.\n\n
    :type image: buffer|numpy.ndarray|PIL.Image.Image\n
    :param image: The image object to decode. The image can be either:\n
        - a buffer with the correct shape, use .cast on memory view to convert\n
        - a numpy array containing image either in grayscale (1 byte per pixel) or BGR mode (3 bytes per pixel)\n
        - a PIL Image\n
        - a QtGui.QImage\n
        - a zxingcpp.ImageView object, which is effectively a memory view but with custom strides and ImageFormat\n
    :type formats: zxing.BarcodeFormat|zxing.BarcodeFormats\n
    :param formats: the format(s) to decode. If ``None``, decode all formats.\n
    :type try_rotate: bool\n
    :param try_rotate: if ``True`` (the default), decoder searches for barcodes in any direction; \n
        if ``False``, it will not search for 90° / 270° rotated barcodes.\n
    :type try_downscale: bool\n
    :param try_downscale: if ``True`` (the default), decoder also scans downscaled versions of the input; \n
        if ``False``, it will only search in the resolution provided.\n
    :type try_invert: bool\n
    :param try_invert: if ``True`` (the default), decoder also tries inverted (light on dark) barcodes.\n
    :type text_mode: zxing.TextMode\n
    :param text_mode: specifies the TextMode that governs how the raw bytes content is transcoded to text.\n
        Defaults to :py:attr:`zxing.TextMode.HRI`.
    :type binarizer: zxing.Binarizer\n
    :param binarizer: the binarizer used to convert image before decoding barcodes.\n
        Defaults to :py:attr:`zxing.Binarizer.LocalAverage`.
    :type is_pure: bool\n
    :param is_pure: Set to True if the input contains nothing but a perfectly aligned barcode (generated image).\n
        Speeds up detection in that case. Default is False.
    :type ean_add_on_symbol: zxing.EanAddOnSymbol\n
    :param ean_add_on_symbol: Specify whether to Ignore, Read or Require EAN-2/5 add-on symbols while scanning \n
        EAN/UPC codes. Default is ``Ignore``.\n
    :type return_errors: bool\n
    :param return_errors: Set to True to return the barcodes with errors as well (e.g. checksum errors); see ``Barcode.error``.\n
        Default is False.
    :rtype: zxingcpp.Barcode\n
    :return: a Barcode if found, None otherwise
    """

def read_barcodes(
    image: object,
    formats: BarcodeFormatsLike = BarcodeFormats(),
    try_rotate: bool = True,
    try_downscale: bool = True,
    try_invert: bool = True,
    text_mode: TextMode = TextMode.HRI,
    binarizer: Binarizer = Binarizer.LocalAverage,
    is_pure: bool = False,
    ean_add_on_symbol: EanAddOnSymbol = EanAddOnSymbol.Ignore,
    return_errors: bool = False,
) -> list[Barcode]:
    """
    Read (decode) multiple barcodes from a numpy BGR or grayscale image array or from a PIL image.\n\n
    :type image: buffer|numpy.ndarray|PIL.Image.Image\n
    :param image: The image object to decode. The image can be either:\n
        - a buffer with the correct shape, use .cast on memoryview to convert\n
        - a numpy array containing image either in grayscale (1 byte per pixel) or BGR mode (3 bytes per pixel)\n
        - a PIL Image\n
        - a QtGui.QImage\n
        - a zxingcpp.ImageView object, which is effectively a memory view but with custom strides and ImageFormat\n
    :type formats: zxing.BarcodeFormat|zxing.BarcodeFormats\n
    :param formats: the format(s) to decode. If ``None``, decode all formats.\n
    :type try_rotate: bool\n
    :param try_rotate: if ``True`` (the default), decoder searches for barcodes in any direction; \n
        if ``False``, it will not search for 90° / 270° rotated barcodes.\n
    :type try_downscale: bool\n
    :param try_downscale: if ``True`` (the default), decoder also scans downscaled versions of the input; \n
        if ``False``, it will only search in the resolution provided.\n
    :type try_invert: bool\n
    :param try_invert: if ``True`` (the default), decoder also tries inverted (light on dark) barcodes.\n
    :type text_mode: zxing.TextMode\n
    :param text_mode: specifies the TextMode that governs how the raw bytes content is transcoded to text.\n
        Defaults to :py:attr:`zxing.TextMode.HRI`.
    :type binarizer: zxing.Binarizer\n
    :param binarizer: the binarizer used to convert image before decoding barcodes.\n
        Defaults to :py:attr:`zxing.Binarizer.LocalAverage`.
    :type is_pure: bool\n
    :param is_pure: Set to True if the input contains nothing but a perfectly aligned barcode (generated image).\n
        Speeds up detection in that case. Default is False.
    :type ean_add_on_symbol: zxing.EanAddOnSymbol\n
    :param ean_add_on_symbol: Specify whether to Ignore, Read or Require EAN-2/5 add-on symbols while scanning \n
        EAN/UPC codes. Default is ``Ignore``.\n
    :type return_errors: bool\n
    :param return_errors: Set to True to return the barcodes with errors as well (e.g. checksum errors); see ``Barcode.error``.\n
        Default is False.\n
    :rtype: list[zxingcpp.Barcode]\n
    :return: a list of Barcodes, the list is empty if none is found
    """

class Image:
    @property
    def __array_interface__(self) -> dict[str, int]: ...  # F IXME typeddict
    @property
    def shape(self) -> tuple[int, int]: ...

def create_barcode(
    content: str | bytes, format: BarcodeFormat, **kwargs: Any
) -> Barcode: ...
def write_barcode_to_image(
    barcode: Barcode, scale: int = 1, add_hrt: bool = False, add_quiet_zone: bool = True
) -> Image: ...
def write_barcode_to_svg(
    barcode: Barcode, scale: int = 1, add_hrt: bool = False, add_quiet_zone: bool = True
) -> Image: ...

class ImageView:
    def __init__(
        self,
        buffer: Buffer,
        width: int,
        height: int,
        format: ImageFormat,
        row_stride: int = 0,
        pix_stride: int = 0,
    ) -> None: ...
    @property
    def format(self) -> BarcodeFormat: ...

@deprecated("Use Image instead.")
class Bitmap(Image): ...

@deprecated(
    "write_barcode() is deprecated, use create_barcode() and write_barcode_to_image() instead."
)
def write_barcode(
    format: BarcodeFormat,
    text: str | bytes,
    width: int = 0,
    height: int = 0,
    quiet_zone: int = -1,
    ec_level: int = -1,
) -> Image:
    """
    Write (encode) a text into a barcode and return 8-bit grayscale bitmap buffer\n\n
    :type format: zxing.BarcodeFormat\n
    :param format: format of the barcode to create\n
    :type text: str|bytes\n
    :param text: the text/content of the barcode. A str is encoded as utf8 text and bytes as binary data\n
    :type width: int\n
    :param width: width (in pixels) of the barcode to create. If undefined (or set to 0), barcode will be\n
        created with the minimum possible width\n
    :type height: int\n
    :param height: height (in pixels) of the barcode to create. If undefined (or set to 0), barcode will be\n
        created with the minimum possible height\n
    :type quiet_zone: int\n
    :param quiet_zone: minimum size (in pixels) of the quiet zone around barcode. If undefined (or set to -1), \n
        the minimum quiet zone of respective barcode is used.
    :type ec_level: int\n
    :param ec_level: error correction level of the barcode (Used for Aztec, PDF417, and QRCode only).\n
    :rtype: zxingcpp.Bitmap\n
    """

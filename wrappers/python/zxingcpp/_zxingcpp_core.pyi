"""python bindings for zxing-cpp"""

from collections.abc import Iterable, Iterator
import enum
from typing import TypeAlias, overload

import numpy
from numpy.typing import NDArray


class BarcodeFormats:
    @overload
    def __init__(self, arg: BarcodeFormat, /) -> None: ...

    @overload
    def __init__(self, arg: Iterable, /) -> None: ...

    def __repr__(self) -> str: ...

    def __eq__(self, arg: BarcodeFormats, /) -> bool: ...

    def __or__(self, arg: BarcodeFormat, /) -> BarcodeFormats: ...

    def __len__(self) -> int: ...

    def __iter__(self) -> Iterator[BarcodeFormat]: ...

    def __getitem__(self, arg: int, /) -> BarcodeFormat: ...

class BarcodeFormat(enum.IntEnum):
    """Enumeration of zxing supported barcode formats"""

    def __str__(self) -> str: ...

    None = 0

    All = 10794

    AllReadable = 29226

    AllCreatable = 30506

    AllLinear = 27690

    AllMatrix = 27946

    AllGS1 = 18218

    AllRetail = 21034

    AllIndustrial = 18730

    Codabar = 8262

    Code39 = 8257

    Code39Std = 29505

    Code39Ext = 25921

    Code32 = 12865

    PZN = 28737

    Code93 = 8263

    Code128 = 8259

    ITF = 8265

    ITF14 = 13385

    DataBar = 8293

    DataBarOmni = 28517

    DataBarStk = 29541

    DataBarStkOmni = 20325

    DataBarLtd = 27749

    DataBarExp = 25957

    DataBarExpStk = 17765

    EANUPC = 8261

    EAN13 = 12613

    EAN8 = 14405

    EAN5 = 13637

    EAN2 = 12869

    ISBN = 26949

    UPCA = 24901

    UPCE = 25925

    OtherBarcode = 8280

    DXFilmEdge = 30808

    PDF417 = 8268

    CompactPDF417 = 25420

    MicroPDF417 = 27980

    Aztec = 8314

    AztecCode = 25466

    AztecRune = 29306

    QRCode = 8273

    QRCodeModel1 = 12625

    QRCodeModel2 = 12881

    MicroQRCode = 27985

    RMQRCode = 29265

    DataMatrix = 8292

    MaxiCode = 8277

    NONE = 0

    DataBarExpanded = 25957

    DataBarLimited = 27749

    LinearCodes = 27690

    MatrixCodes = 27946

    def __or__(self, arg: BarcodeFormat, /) -> BarcodeFormats: ...

    @property
    def symbology(self) -> BarcodeFormat: ...

None: ErrorType = ErrorType.None

All: BarcodeFormat = BarcodeFormat.All

AllReadable: BarcodeFormat = BarcodeFormat.AllReadable

AllCreatable: BarcodeFormat = BarcodeFormat.AllCreatable

AllLinear: BarcodeFormat = BarcodeFormat.AllLinear

AllMatrix: BarcodeFormat = BarcodeFormat.AllMatrix

AllGS1: BarcodeFormat = BarcodeFormat.AllGS1

AllRetail: BarcodeFormat = BarcodeFormat.AllRetail

AllIndustrial: BarcodeFormat = BarcodeFormat.AllIndustrial

Codabar: BarcodeFormat = BarcodeFormat.Codabar

Code39: BarcodeFormat = BarcodeFormat.Code39

Code39Std: BarcodeFormat = BarcodeFormat.Code39Std

Code39Ext: BarcodeFormat = BarcodeFormat.Code39Ext

Code32: BarcodeFormat = BarcodeFormat.Code32

PZN: BarcodeFormat = BarcodeFormat.PZN

Code93: BarcodeFormat = BarcodeFormat.Code93

Code128: BarcodeFormat = BarcodeFormat.Code128

ITF: BarcodeFormat = BarcodeFormat.ITF

ITF14: BarcodeFormat = BarcodeFormat.ITF14

DataBar: BarcodeFormat = BarcodeFormat.DataBar

DataBarOmni: BarcodeFormat = BarcodeFormat.DataBarOmni

DataBarStk: BarcodeFormat = BarcodeFormat.DataBarStk

DataBarStkOmni: BarcodeFormat = BarcodeFormat.DataBarStkOmni

DataBarLtd: BarcodeFormat = BarcodeFormat.DataBarLtd

DataBarExp: BarcodeFormat = BarcodeFormat.DataBarExp

DataBarExpStk: BarcodeFormat = BarcodeFormat.DataBarExpStk

EANUPC: BarcodeFormat = BarcodeFormat.EANUPC

EAN13: BarcodeFormat = BarcodeFormat.EAN13

EAN8: BarcodeFormat = BarcodeFormat.EAN8

EAN5: BarcodeFormat = BarcodeFormat.EAN5

EAN2: BarcodeFormat = BarcodeFormat.EAN2

ISBN: BarcodeFormat = BarcodeFormat.ISBN

UPCA: BarcodeFormat = BarcodeFormat.UPCA

UPCE: BarcodeFormat = BarcodeFormat.UPCE

OtherBarcode: BarcodeFormat = BarcodeFormat.OtherBarcode

DXFilmEdge: BarcodeFormat = BarcodeFormat.DXFilmEdge

PDF417: BarcodeFormat = BarcodeFormat.PDF417

CompactPDF417: BarcodeFormat = BarcodeFormat.CompactPDF417

MicroPDF417: BarcodeFormat = BarcodeFormat.MicroPDF417

Aztec: BarcodeFormat = BarcodeFormat.Aztec

AztecCode: BarcodeFormat = BarcodeFormat.AztecCode

AztecRune: BarcodeFormat = BarcodeFormat.AztecRune

QRCode: BarcodeFormat = BarcodeFormat.QRCode

QRCodeModel1: BarcodeFormat = BarcodeFormat.QRCodeModel1

QRCodeModel2: BarcodeFormat = BarcodeFormat.QRCodeModel2

MicroQRCode: BarcodeFormat = BarcodeFormat.MicroQRCode

RMQRCode: BarcodeFormat = BarcodeFormat.RMQRCode

DataMatrix: BarcodeFormat = BarcodeFormat.DataMatrix

MaxiCode: BarcodeFormat = BarcodeFormat.MaxiCode

class Binarizer(enum.Enum):
    """Enumeration of binarizers used before decoding images"""

    BoolCast = 3

    FixedThreshold = 2

    GlobalHistogram = 1

    LocalAverage = 0

BoolCast: Binarizer = Binarizer.BoolCast

FixedThreshold: Binarizer = Binarizer.FixedThreshold

GlobalHistogram: Binarizer = Binarizer.GlobalHistogram

LocalAverage: Binarizer = Binarizer.LocalAverage

class EanAddOnSymbol(enum.Enum):
    """Enumeration of options for EAN-2/5 add-on symbols check"""

    Ignore = 0
    """Ignore any Add-On symbol during read/scan"""

    Read = 1
    """Read EAN-2/EAN-5 Add-On symbol if found"""

    Require = 2
    """Require EAN-2/EAN-5 Add-On symbol to be present"""

Ignore: EanAddOnSymbol = EanAddOnSymbol.Ignore

Read: EanAddOnSymbol = EanAddOnSymbol.Read

Require: EanAddOnSymbol = EanAddOnSymbol.Require

class ContentType(enum.Enum):
    """Enumeration of content types"""

    Text = 0

    Binary = 1

    Mixed = 2

    GS1 = 3

    ISO15434 = 4

    UnknownECI = 5

Text: ContentType = ContentType.Text

Binary: ContentType = ContentType.Binary

Mixed: ContentType = ContentType.Mixed

GS1: ContentType = ContentType.GS1

ISO15434: ContentType = ContentType.ISO15434

UnknownECI: ContentType = ContentType.UnknownECI

class TextMode(enum.Enum):
    Plain = 0
    """
    bytes() transcoded to unicode based on ECI info or guessed charset (the default mode prior to 2.0)
    """

    ECI = 1
    """
    standard content following the ECI protocol with every character set ECI segment transcoded to unicode
    """

    HRI = 2
    """Human Readable Interpretation (dependent on the ContentType)"""

    Escaped = 3
    """
    Use the EscapeNonGraphical() function (e.g. ASCII 29 will be transcoded to '<GS>'
    """

    Hex = 4
    """bytes() transcoded to ASCII string of HEX values"""

    HexECI = 5
    """bytesECI() transcoded to ASCII string of HEX values"""

Plain: TextMode = TextMode.Plain

ECI: TextMode = TextMode.ECI

HRI: TextMode = TextMode.HRI

Escaped: TextMode = TextMode.Escaped

Hex: TextMode = TextMode.Hex

HexECI: TextMode = TextMode.HexECI

class ImageFormat(enum.Enum):
    """Enumeration of image formats supported by read_barcodes"""

    Lum = 16777216

    LumA = 33554432

    RGB = 50331906

    BGR = 50462976

    RGBA = 67109122

    ARGB = 67174915

    BGRA = 67240192

    ABGR = 67305985

Lum: ImageFormat = ImageFormat.Lum

LumA: ImageFormat = ImageFormat.LumA

RGB: ImageFormat = ImageFormat.RGB

BGR: ImageFormat = ImageFormat.BGR

RGBA: ImageFormat = ImageFormat.RGBA

ARGB: ImageFormat = ImageFormat.ARGB

BGRA: ImageFormat = ImageFormat.BGRA

ABGR: ImageFormat = ImageFormat.ABGR

class Point:
    """Represents the coordinates of a point in an image"""

    @property
    def x(self) -> int:
        """
        :return: horizontal coordinate of the point
        :rtype: int
        """

    @property
    def y(self) -> int:
        """
        :return: vertical coordinate of the point
        :rtype: int
        """

class Position:
    """The position of a decoded symbol"""

    @property
    def top_left(self) -> Point:
        """
        :return: coordinate of the symbol's top-left corner
        :rtype: zxingcpp.Point
        """

    @property
    def top_right(self) -> Point:
        """
        :return: coordinate of the symbol's top-right corner
        :rtype: zxingcpp.Point
        """

    @property
    def bottom_left(self) -> Point:
        """
        :return: coordinate of the symbol's bottom-left corner
        :rtype: zxingcpp.Point
        """

    @property
    def bottom_right(self) -> Point:
        """
        :return: coordinate of the symbol's bottom-right corner
        :rtype: zxingcpp.Point
        """

    def __str__(self) -> str: ...

class ErrorType(enum.Enum):
    None = 0
    """No error"""

    Format = 1
    """Data format error"""

    Checksum = 2
    """Checksum error"""

    Unsupported = 3
    """Unsupported content error"""

Format: ErrorType = ErrorType.Format

Checksum: ErrorType = ErrorType.Checksum

Unsupported: ErrorType = ErrorType.Unsupported

class Error:
    """Barcode reading error"""

    @property
    def type(self) -> ErrorType:
        """
        :return: Error type
        :rtype: zxingcpp.ErrorType
        """

    @property
    def message(self) -> str:
        """
        :return: Error message
        :rtype: str
        """

    def __str__(self) -> str: ...

class Barcode:
    """The Barcode class"""

    @property
    def valid(self) -> bool:
        """
        :return: whether or not barcode is valid (i.e. a symbol was found and decoded)
        :rtype: bool
        """

    @property
    def text(self) -> str:
        """
        :return: text of the decoded symbol (see also TextMode parameter)
        :rtype: str
        """

    @property
    def bytes(self) -> bytes:
        """
        :return: uninterpreted bytes of the decoded symbol
        :rtype: bytes
        """

    @property
    def format(self) -> BarcodeFormat:
        """
        :return: decoded symbol format
        :rtype: zxingcpp.BarcodeFormat
        """

    @property
    def symbology(self) -> BarcodeFormat:
        """
        :return: decoded symbol symbology
        :rtype: zxingcpp.BarcodeFormat
        """

    @property
    def symbology_identifier(self) -> str:
        """
        :return: decoded symbology identifier
        :rtype: str
        """

    @property
    def ec_level(self) -> str:
        """
        :return: error correction level of the symbol (empty string if not applicable)
        :rtype: str
        """

    @property
    def content_type(self) -> ContentType:
        """
        :return: content type of symbol
        :rtype: zxingcpp.ContentType
        """

    @property
    def position(self) -> Position:
        """
        :return: position of the decoded symbol
        :rtype: zxingcpp.Position
        """

    @property
    def orientation(self) -> int:
        """
        :return: orientation (in degree) of the decoded symbol
        :rtype: int
        """

    @property
    def error(self) -> Error | None:
        """
        :return: Error code or None
        :rtype: zxingcpp.Error
        """

    @property
    def extra(self) -> object:
        """
        :return: Symbology specific extra information as a Python dictionary (might be empty)
        :rtype: dict
        """

    def to_image(self, scale: int = 1, add_hrt: bool = False, add_quiet_zones: bool = True) -> Image: ...

    def to_svg(self, scale: int = 1, add_hrt: bool = False, add_quiet_zones: bool = True) -> str: ...

Result: TypeAlias = Barcode

class Image:
    def __buffer__(self, flags, /):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """

    def __release_buffer__(self, buffer, /):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """

    @property
    def __array_interface__(self) -> dict: ...

    @property
    def shape(self) -> tuple: ...

def create_barcode(arg0: object, arg1: BarcodeFormat, /, **kwargs) -> Barcode: ...

def write_barcode_to_image(barcode: Barcode, scale: int = 1, add_hrt: bool = False, add_quiet_zones: bool = True) -> Image: ...

def write_barcode_to_svg(barcode: Barcode, scale: int = 1, add_hrt: bool = False, add_quiet_zones: bool = True) -> str: ...

class ImageView:
    def __init__(self, buffer: object, width: int, height: int, format: ImageFormat, row_stride: int = 0, pix_stride: int = 0) -> None: ...

    def __buffer__(self, flags, /):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """

    def __release_buffer__(self, buffer, /):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """

    @property
    def format(self) -> ImageFormat: ...

Bitmap: TypeAlias = Image

def barcode_format_from_str(str: str) -> BarcodeFormat:
    """
    Convert string to BarcodeFormat

    :type str: str
    :param str: string representing barcode format
    :return: corresponding barcode format
    :rtype: zxingcpp.BarcodeFormat
    """

def barcode_formats_from_str(str: str) -> BarcodeFormats:
    """
    Convert string to BarcodeFormats

    :type str: str
    :param str: string representing a list of barcodes formats
    :return: corresponding barcode formats
    :rtype: zxingcpp.BarcodeFormats
    """

def barcode_formats_list(filter: BarcodeFormats = ...) -> BarcodeFormats:
    """
    Returns a list of available/supported barcode formats, optionally filtered by the provided format(s).

    :type filter: zxingcpp.BarcodeFormats
    :param filter: the BarcodeFormat(s) to filter by
    :return: list of available/supported barcode formats (optionally filtered)
    :rtype: list[zxingcpp.BarcodeFormat]
    """

@overload
def read_barcode(image: NDArray[numpy.uint8], formats: BarcodeFormats = ..., try_rotate: bool = True, try_downscale: bool = True, try_invert: bool = True, text_mode: TextMode = TextMode.HRI, binarizer: Binarizer = Binarizer.LocalAverage, is_pure: bool = False, ean_add_on_symbol: EanAddOnSymbol = EanAddOnSymbol.Ignore, return_errors: bool = False) -> Barcode | None: ...

@overload
def read_barcode(image: object, formats: BarcodeFormats = ..., try_rotate: bool = True, try_downscale: bool = True, try_invert: bool = True, text_mode: TextMode = TextMode.HRI, binarizer: Binarizer = Binarizer.LocalAverage, is_pure: bool = False, ean_add_on_symbol: EanAddOnSymbol = EanAddOnSymbol.Ignore, return_errors: bool = False) -> Barcode | None:
    """
    Read (decode) a barcode from a numpy BGR or grayscale image array or from a PIL image.

    :type image: buffer|numpy.ndarray|PIL.Image.Image
    :param image: The image object to decode. The image can be either:
      - a buffer with the correct shape, use .cast on memory view to convert
      - a numpy array containing image either in grayscale (1 byte per pixel) or BGR mode (3 bytes per pixel)
      - a PIL Image
      - a QtGui.QImage
      - a zxingcpp.ImageView object, which is effectively a memory view but with custom strides and ImageFormat
    :type formats: zxing.BarcodeFormat|zxing.BarcodeFormats
    :param formats: the format(s) to decode. If ``None``, decode all formats.
    :type try_rotate: bool
    :param try_rotate: if ``True`` (the default), decoder searches for barcodes in any direction; 
      if ``False``, it will not search for 90° / 270° rotated barcodes.
    :type try_downscale: bool
    :param try_downscale: if ``True`` (the default), decoder also scans downscaled versions of the input; 
      if ``False``, it will only search in the resolution provided.
    :type try_invert: bool
    :param try_invert: if ``True`` (the default), decoder also tries inverted (light on dark) barcodes.
    :type text_mode: zxing.TextMode
    :param text_mode: specifies the TextMode that governs how the raw bytes content is transcoded to text.
      Defaults to :py:attr:`zxing.TextMode.HRI`.:type binarizer: zxing.Binarizer
    :param binarizer: the binarizer used to convert image before decoding barcodes.
      Defaults to :py:attr:`zxing.Binarizer.LocalAverage`.:type is_pure: bool
    :param is_pure: Set to True if the input contains nothing but a perfectly aligned barcode (generated image).
      Speeds up detection in that case. Default is False.:type ean_add_on_symbol: zxing.EanAddOnSymbol
    :param ean_add_on_symbol: Specify whether to Ignore, Read or Require EAN-2/5 add-on symbols while scanning 
      EAN/UPC codes. Default is ``Ignore``.
    :type return_errors: bool
    :param return_errors: Set to True to return the barcodes with errors as well (e.g. checksum errors); see ``Barcode.error``.
     Default is False.:rtype: zxingcpp.Barcode
    :return: a Barcode if found, None otherwise
    """

@overload
def read_barcodes(image: NDArray[numpy.uint8], formats: BarcodeFormats = ..., try_rotate: bool = True, try_downscale: bool = True, try_invert: bool = True, text_mode: TextMode = TextMode.HRI, binarizer: Binarizer = Binarizer.LocalAverage, is_pure: bool = False, ean_add_on_symbol: EanAddOnSymbol = EanAddOnSymbol.Ignore, return_errors: bool = False) -> list[Barcode]: ...

@overload
def read_barcodes(image: object, formats: BarcodeFormats = ..., try_rotate: bool = True, try_downscale: bool = True, try_invert: bool = True, text_mode: TextMode = TextMode.HRI, binarizer: Binarizer = Binarizer.LocalAverage, is_pure: bool = False, ean_add_on_symbol: EanAddOnSymbol = EanAddOnSymbol.Ignore, return_errors: bool = False) -> list[Barcode]:
    """
    Read (decode) multiple barcodes from a numpy BGR or grayscale image array or from a PIL image.

    :type image: buffer|numpy.ndarray|PIL.Image.Image
    :param image: The image object to decode. The image can be either:
      - a buffer with the correct shape, use .cast on memoryview to convert
      - a numpy array containing image either in grayscale (1 byte per pixel) or BGR mode (3 bytes per pixel)
      - a PIL Image
      - a QtGui.QImage
      - a zxingcpp.ImageView object, which is effectively a memory view but with custom strides and ImageFormat
    :type formats: zxing.BarcodeFormat|zxing.BarcodeFormats
    :param formats: the format(s) to decode. If ``None``, decode all formats.
    :type try_rotate: bool
    :param try_rotate: if ``True`` (the default), decoder searches for barcodes in any direction; 
      if ``False``, it will not search for 90° / 270° rotated barcodes.
    :type try_downscale: bool
    :param try_downscale: if ``True`` (the default), decoder also scans downscaled versions of the input; 
      if ``False``, it will only search in the resolution provided.
    :type try_invert: bool
    :param try_invert: if ``True`` (the default), decoder also tries inverted (light on dark) barcodes.
    :type text_mode: zxing.TextMode
    :param text_mode: specifies the TextMode that governs how the raw bytes content is transcoded to text.
      Defaults to :py:attr:`zxing.TextMode.HRI`.:type binarizer: zxing.Binarizer
    :param binarizer: the binarizer used to convert image before decoding barcodes.
      Defaults to :py:attr:`zxing.Binarizer.LocalAverage`.:type is_pure: bool
    :param is_pure: Set to True if the input contains nothing but a perfectly aligned barcode (generated image).
      Speeds up detection in that case. Default is False.:type ean_add_on_symbol: zxing.EanAddOnSymbol
    :param ean_add_on_symbol: Specify whether to Ignore, Read or Require EAN-2/5 add-on symbols while scanning 
      EAN/UPC codes. Default is ``Ignore``.
    :type return_errors: bool
    :param return_errors: Set to True to return the barcodes with errors as well (e.g. checksum errors); see ``Barcode.error``.
     Default is False.
    :rtype: list[zxingcpp.Barcode]
    :return: a list of Barcodes, the list is empty if none is found
    """

def write_barcode(format: BarcodeFormat, text: object, width: int = 0, height: int = 0, quiet_zone: int = -1, ec_level: int = -1) -> Image:
    """
    Write (encode) a text into a barcode and return 8-bit grayscale bitmap buffer

    :type format: zxing.BarcodeFormat
    :param format: format of the barcode to create
    :type text: str|bytes
    :param text: the text/content of the barcode. A str is encoded as utf8 text and bytes as binary data
    :type width: int
    :param width: width (in pixels) of the barcode to create. If undefined (or set to 0), barcode will be
      created with the minimum possible width
    :type height: int
    :param height: height (in pixels) of the barcode to create. If undefined (or set to 0), barcode will be
      created with the minimum possible height
    :type quiet_zone: int
    :param quiet_zone: minimum size (in pixels) of the quiet zone around barcode. If undefined (or set to -1), 
      the minimum quiet zone of respective barcode is used.:type ec_level: int
    :param ec_level: error correction level of the barcode (Used for Aztec, PDF417, and QRCode only).
    :rtype: zxingcpp.Bitmap
    """

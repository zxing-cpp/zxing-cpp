#pragma once

namespace ZXing {

class DecodeHints;
class BinaryBitmap;
class Result;

/**
* Implementations of this interface can decode an image of a barcode in some format into
* the String it encodes. For example, {@link com.google.zxing.qrcode.QRCodeReader} can
* decode a QR code. The decoder may optionally receive hints from the caller which may help
* it decode more quickly or accurately.
*
* See {@link MultiFormatReader}, which attempts to determine what barcode
* format is present within the image as well, and then decodes it accordingly.
*
* @author Sean Owen
* @author dswitkin@google.com (Daniel Switkin)
*/
class Reader
{
public:
	virtual ~Reader() {}

	/**
	* Locates and decodes a barcode in some format within an image. This method also accepts
	* hints, each possibly associated to some data, which may help the implementation decode.
	*
	* @param image image of barcode to decode
	* @param hints passed as a {@link java.util.Map} from {@link DecodeHintType}
	* to arbitrary data. The
	* meaning of the data depends upon the hint type. The implementation may or may not do
	* anything with these hints.
	* @return String which the barcode encodes
	* @throws NotFoundException if no potential barcode is found
	* @throws ChecksumException if a potential barcode is found but does not pass its checksum
	* @throws FormatException if a potential barcode is found but format is invalid
	*/
	virtual Result decode(const BinaryBitmap& image, const DecodeHints* hints = nullptr) const = 0;
};

}
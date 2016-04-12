#include "LuminanceSource.h"
#include "ByteArray.h"

#include <stdexcept>

namespace ZXing {

namespace {

/**
* A wrapper implementation of {@link LuminanceSource} which inverts the luminances it returns -- black becomes
* white and vice versa, and each value becomes (255-value).
*
* @author Sean Owen
*/
class InvertedLuminanceSource : public LuminanceSource
{
	std::shared_ptr<LuminanceSource> _src;

public:
	explicit InvertedLuminanceSource(const std::shared_ptr<LuminanceSource>& src) : LuminanceSource(src->width(), src->height()), _src(src) {}

	virtual ByteArray row(int y) const override
	{
		auto r = _src->row(y);
		int w = width();
		for (int i = 0; i < w; i++) {
			r[i] = 255 - r[i];
		}
		return r;
	}

	virtual ByteArray matrix() const override
	{
		auto m = _src->matrix();
		int length = width() * height();
		for (int i = 0; i < length; i++) {
			m[i] = 255 - m[i];
		}
		return m;
	}

	virtual bool canCrop() const override
	{
		return _src->canCrop();
	}

	virtual std::shared_ptr<LuminanceSource> cropped(int left, int top, int width, int height) const override
	{
		return CreateInverted(_src->cropped(left, top, width, height));
	}

	virtual bool canRotate() const override
	{
		return _src->canRotate();
	}

	virtual std::shared_ptr<LuminanceSource> rotatedCCW90() const override
	{
		return CreateInverted(_src->rotatedCCW90());
	}

	virtual std::shared_ptr<LuminanceSource> rotatedCCW45() const override
	{
		return CreateInverted(_src->rotatedCCW45());
	}

protected:
	virtual std::shared_ptr<LuminanceSource> getInverted() const override
	{
		return _src;
	}

}; // InvertedLuminanceSource

} // anonymous


LuminanceSource::~LuminanceSource()
{
}

bool
LuminanceSource::canCrop() const
{
	return false;
}

std::shared_ptr<LuminanceSource>
LuminanceSource::cropped(int left, int top, int width, int height) const
{
	throw std::runtime_error("This luminance source does not support cropping.");
}

bool
LuminanceSource::canRotate() const
{
	return false;
}

/**
* Returns a new object with rotated image data by 90 degrees counterclockwise.
* Only callable if {@link #isRotateSupported()} is true.
*
* @return A rotated version of this object.
*/
std::shared_ptr<LuminanceSource>
LuminanceSource::rotatedCCW90() const
{
	throw std::runtime_error("This luminance source does not support rotation by 90 degrees.");
}

/**
* Returns a new object with rotated image data by 45 degrees counterclockwise.
* Only callable if {@link #isRotateSupported()} is true.
*
* @return A rotated version of this object.
*/
std::shared_ptr<LuminanceSource>
LuminanceSource::rotatedCCW45() const
{
	throw std::runtime_error("This luminance source does not support rotation by 45 degrees.");
}

//@Override
//public final String toString() {
//	byte[] row = new byte[width];
//	StringBuilder result = new StringBuilder(height * (width + 1));
//	for (int y = 0; y < height; y++) {
//		row = getRow(y, row);
//		for (int x = 0; x < width; x++) {
//			int luminance = row[x] & 0xFF;
//			char c;
//			if (luminance < 0x40) {
//				c = '#';
//			}
//			else if (luminance < 0x80) {
//				c = '+';
//			}
//			else if (luminance < 0xC0) {
//				c = '.';
//			}
//			else {
//				c = ' ';
//			}
//			result.append(c);
//		}
//		result.append('\n');
//	}
//	return result.toString();
//}

std::shared_ptr<LuminanceSource>
LuminanceSource::getInverted() const
{
	return nullptr;
}

std::shared_ptr<LuminanceSource>
LuminanceSource::CreateInverted(const std::shared_ptr<LuminanceSource>& src)
{
	auto result = src->getInverted();
	if (result == nullptr)
		result = std::make_shared<InvertedLuminanceSource>(src);
	return result;
}

}
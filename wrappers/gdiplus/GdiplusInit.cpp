#include "GdiplusInit.h"
#include <gdiplus.h>
#include <stdexcept>

namespace ZXing {

GdiplusInit::GdiplusInit()
{
	Gdiplus::GdiplusStartupInput input;
	if (Gdiplus::GdiplusStartup(&_token, &input, NULL) != Gdiplus::Ok)
		throw std::runtime_error("Unable to initialize GDI+ runtimes");
}
	
GdiplusInit::~GdiplusInit()
{
	Gdiplus::GdiplusShutdown(_token);
}

} // ZXing

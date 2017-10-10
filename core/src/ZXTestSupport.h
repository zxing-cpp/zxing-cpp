#pragma once

#ifdef ZXING_BUILD_FOR_TEST
#define ZXING_EXPORT_TEST_ONLY
#else
#define ZXING_EXPORT_TEST_ONLY static
#endif

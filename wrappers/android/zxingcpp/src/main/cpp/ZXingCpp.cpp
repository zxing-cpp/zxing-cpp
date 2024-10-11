/*
* Copyright 2021 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ReadBarcode.h"

#include <android/bitmap.h>
#include <android/log.h>
#include <chrono>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>

using namespace ZXing;
using namespace std::string_literals;

#define PACKAGE "zxingcpp/BarcodeReader$"

#define ZX_LOG_TAG "zxingcpp"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, ZX_LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, ZX_LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, ZX_LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, ZX_LOG_TAG, __VA_ARGS__)

static const char* JavaBarcodeFormatName(BarcodeFormat format)
{
	// These have to be the names of the enum constants in the kotlin code.
	switch (format) {
	case BarcodeFormat::None: return "NONE";
	case BarcodeFormat::Aztec: return "AZTEC";
	case BarcodeFormat::Codabar: return "CODABAR";
	case BarcodeFormat::Code39: return "CODE_39";
	case BarcodeFormat::Code93: return "CODE_93";
	case BarcodeFormat::Code128: return "CODE_128";
	case BarcodeFormat::DataMatrix: return "DATA_MATRIX";
	case BarcodeFormat::EAN8: return "EAN_8";
	case BarcodeFormat::EAN13: return "EAN_13";
	case BarcodeFormat::ITF: return "ITF";
	case BarcodeFormat::MaxiCode: return "MAXICODE";
	case BarcodeFormat::PDF417: return "PDF_417";
	case BarcodeFormat::QRCode: return "QR_CODE";
	case BarcodeFormat::MicroQRCode: return "MICRO_QR_CODE";
	case BarcodeFormat::RMQRCode: return "RMQR_CODE";
	case BarcodeFormat::DataBar: return "DATA_BAR";
	case BarcodeFormat::DataBarExpanded: return "DATA_BAR_EXPANDED";
	case BarcodeFormat::DataBarLimited: return "DATA_BAR_LIMITED";
	case BarcodeFormat::DXFilmEdge: return "DX_FILM_EDGE";
	case BarcodeFormat::UPCA: return "UPC_A";
	case BarcodeFormat::UPCE: return "UPC_E";
	default: throw std::invalid_argument("Invalid BarcodeFormat");
	}
}

static const char* JavaContentTypeName(ContentType contentType)
{
	// These have to be the names of the enum constants in the kotlin code.
	switch (contentType) {
	case ContentType::Text: return "TEXT";
	case ContentType::Binary: return "BINARY";
	case ContentType::Mixed: return "MIXED";
	case ContentType::GS1: return "GS1";
	case ContentType::ISO15434: return "ISO15434";
	case ContentType::UnknownECI: return "UNKNOWN_ECI";
	default: throw std::invalid_argument("Invalid contentType");
	}
}

static const char* JavaErrorTypeName(Error::Type errorType)
{
	// These have to be the names of the enum constants in the kotlin code.
	switch (errorType) {
	case Error::Type::Format: return "FORMAT";
	case Error::Type::Checksum: return "CHECKSUM";
	case Error::Type::Unsupported: return "UNSUPPORTED";
	default: throw std::invalid_argument("Invalid errorType");
	}
}

inline constexpr auto hash(std::string_view sv)
{
	unsigned int hash = 5381;
	for (unsigned char c : sv)
		hash = ((hash << 5) + hash) ^ c;
	return hash;
}

inline constexpr auto operator "" _h(const char* str, size_t len){ return hash({str, len}); }

static EanAddOnSymbol EanAddOnSymbolFromString(std::string_view name)
{
	switch (hash(name)) {
		case "IGNORE"_h :  return EanAddOnSymbol::Ignore;
		case "READ"_h :    return EanAddOnSymbol::Read;
		case "REQUIRE"_h : return EanAddOnSymbol::Require;
		default: throw std::invalid_argument("Invalid eanAddOnSymbol name");
	}
}

static Binarizer BinarizerFromString(std::string_view name)
{
	switch (hash(name)) {
		case "LOCAL_AVERAGE"_h :    return Binarizer::LocalAverage;
		case "GLOBAL_HISTOGRAM"_h : return Binarizer::GlobalHistogram;
		case "FIXED_THRESHOLD"_h :  return Binarizer::FixedThreshold;
		case "BOOL_CAST"_h :        return Binarizer::BoolCast;
		default: throw std::invalid_argument("Invalid binarizer name");
	}
}

static TextMode TextModeFromString(std::string_view name)
{
	switch (hash(name)) {
		case "PLAIN"_h :   return TextMode::Plain;
		case "ECI"_h :     return TextMode::ECI;
		case "HRI"_h :     return TextMode::HRI;
		case "HEX"_h :     return TextMode::Hex;
		case "ESCAPED"_h : return TextMode::Escaped;
		default: throw std::invalid_argument("Invalid textMode name");
	}
}

static jstring ThrowJavaException(JNIEnv* env, const char* message)
{
	//	if (env->ExceptionCheck())
	//		return 0;
	jclass cls = env->FindClass("java/lang/RuntimeException");
	env->ThrowNew(cls, message);
	return nullptr;
}

jstring C2JString(JNIEnv* env, const std::string& str)
{
	return env->NewStringUTF(str.c_str());
}

std::string J2CString(JNIEnv* env, jstring str)
{
	// Buffer size must be in bytes.
	const jsize size = env->GetStringUTFLength(str);
	std::string res(size, 0);

	// Translates 'len' number of Unicode characters into modified
	// UTF-8 encoding and place the result in the given buffer.
	const jsize len = env->GetStringLength(str);
	env->GetStringUTFRegion(str, 0, len, res.data());

	return res;
}

static jobject NewPosition(JNIEnv* env, const Position& position)
{
	jclass clsPosition = env->FindClass(PACKAGE "Position");
	jclass clsPoint = env->FindClass("android/graphics/Point");
	jmethodID midPointInit= env->GetMethodID(clsPoint, "<init>", "(II)V");
	auto NewPoint = [&](const PointI& point) {
		return env->NewObject(clsPoint, midPointInit, point.x, point.y);
	};
	jmethodID midPositionInit= env->GetMethodID(
		clsPosition, "<init>",
		"(Landroid/graphics/Point;"
		"Landroid/graphics/Point;"
		"Landroid/graphics/Point;"
		"Landroid/graphics/Point;"
		"D)V");
	return env->NewObject(
			clsPosition, midPositionInit,
			NewPoint(position[0]),
			NewPoint(position[1]),
			NewPoint(position[2]),
			NewPoint(position[3]),
			position.orientation());
}

static jbyteArray NewByteArray(JNIEnv* env, const std::vector<uint8_t>& byteArray)
{
	auto size = static_cast<jsize>(byteArray.size());
	jbyteArray res = env->NewByteArray(size);
	env->SetByteArrayRegion(res, 0, size, reinterpret_cast<const jbyte*>(byteArray.data()));
	return res;
}

static jobject NewEnum(JNIEnv* env, const char* value, const char* type)
{
	auto className = PACKAGE ""s + type;
	jclass cls = env->FindClass(className.c_str());
	jfieldID fidCT = env->GetStaticFieldID(cls, value, ("L" + className + ";").c_str());
	return env->GetStaticObjectField(cls, fidCT);
}

static jobject NewError(JNIEnv* env, const Error& error)
{
	jclass cls = env->FindClass(PACKAGE "Error");
	jmethodID midInit = env->GetMethodID(cls, "<init>", "(L" PACKAGE "ErrorType;" "Ljava/lang/String;)V");
	return env->NewObject(cls, midInit, NewEnum(env, JavaErrorTypeName(error.type()), "ErrorType"), C2JString(env, error.msg()));
}

static jobject NewResult(JNIEnv* env, const Barcode& result)
{
	jclass cls = env->FindClass(PACKAGE "Result");
	jmethodID midInit = env->GetMethodID(
		cls, "<init>",
		"(L" PACKAGE "Format;"
		"[B"
		"Ljava/lang/String;"
		"L" PACKAGE "ContentType;"
		"L" PACKAGE "Position;"
		"I"
		"Ljava/lang/String;"
		"Ljava/lang/String;"
		"I"
		"I"
		"Ljava/lang/String;"
		"Z"
		"I"
		"L" PACKAGE "Error;"
		")V");
	bool valid = result.isValid();
	return env->NewObject(cls, midInit,
		NewEnum(env, JavaBarcodeFormatName(result.format()), "Format"),
		valid ? NewByteArray(env, result.bytes()) : nullptr,
		valid ? C2JString(env, result.text()) : nullptr,
		NewEnum(env, JavaContentTypeName(result.contentType()), "ContentType"),
		NewPosition(env, result.position()),
		result.orientation(),
		valid ? C2JString(env, result.ecLevel()) : nullptr,
		valid ? C2JString(env, result.symbologyIdentifier()) : nullptr,
		result.sequenceSize(),
		result.sequenceIndex(),
		valid ? C2JString(env, result.sequenceId()) : nullptr,
		result.readerInit(),
		result.lineCount(),
		result.error() ? NewError(env, result.error()) : nullptr
	);
}

static jobject Read(JNIEnv *env, jobject thiz, ImageView image, const ReaderOptions& opts)
{
	try {
		auto startTime = std::chrono::high_resolution_clock::now();
		auto barcodes = ReadBarcodes(image, opts);
		auto duration = std::chrono::high_resolution_clock::now() - startTime;
//		LOGD("time: %4d ms\n", (int)std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

		env->SetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "lastReadTime", "I"), time);

		jclass clsList = env->FindClass("java/util/ArrayList");
		jobject objList = env->NewObject(clsList, env->GetMethodID(clsList, "<init>", "()V"));
		if (!barcodes.empty()) {
			jmethodID midAdd = env->GetMethodID(clsList, "add", "(Ljava/lang/Object;)Z");
			for (const auto& barcode: barcodes)
				env->CallBooleanMethod(objList, midAdd, NewResult(env, barcode));
		}
		return objList;
	} catch (const std::exception& e) {
		return ThrowJavaException(env, e.what());
	} catch (...) {
		return ThrowJavaException(env, "Unknown exception");
	}
}

static bool GetBooleanField(JNIEnv* env, jclass cls, jobject opts, const char* name)
{
	return env->GetBooleanField(opts, env->GetFieldID(cls, name, "Z"));
}

static int GetIntField(JNIEnv* env, jclass cls, jobject opts, const char* name)
{
	return env->GetIntField(opts, env->GetFieldID(cls, name, "I"));
}

static std::string GetEnumField(JNIEnv* env, jclass cls, jobject opts, const char* name, const char* type)
{
	auto className = PACKAGE ""s + type;
	jmethodID midName = env->GetMethodID(env->FindClass(className.c_str()), "name", "()Ljava/lang/String;");
	jobject objField = env->GetObjectField(opts, env->GetFieldID(cls, name, ("L"s + className + ";").c_str()));
	return J2CString(env, static_cast<jstring>(env->CallObjectMethod(objField, midName)));
}

static BarcodeFormats GetFormats(JNIEnv* env, jclass clsOptions, jobject opts)
{
	jobject objField = env->GetObjectField(opts, env->GetFieldID(clsOptions, "formats", "Ljava/util/Set;"));
	jmethodID midToArray = env->GetMethodID(env->FindClass("java/util/Set"), "toArray", "()[Ljava/lang/Object;");
	auto objArray = static_cast<jobjectArray>(env->CallObjectMethod(objField, midToArray));
	if (!objArray)
		return {};

	jmethodID midName = env->GetMethodID(env->FindClass(PACKAGE "Format"), "name", "()Ljava/lang/String;");
	BarcodeFormats ret;
	for (int i = 0, size = env->GetArrayLength(objArray); i < size; ++i) {
		auto objName = static_cast<jstring>(env->CallObjectMethod(env->GetObjectArrayElement(objArray, i), midName));
		ret |= BarcodeFormatFromString(J2CString(env, objName));
	}
	return ret;
}

static ReaderOptions CreateReaderOptions(JNIEnv* env, jobject opts)
{
	jclass cls = env->GetObjectClass(opts);
	return ReaderOptions()
		.setFormats(GetFormats(env, cls, opts))
		.setTryHarder(GetBooleanField(env, cls, opts, "tryHarder"))
		.setTryRotate(GetBooleanField(env, cls, opts, "tryRotate"))
		.setTryInvert(GetBooleanField(env, cls, opts, "tryInvert"))
		.setTryDownscale(GetBooleanField(env, cls, opts, "tryDownscale"))
		.setIsPure(GetBooleanField(env, cls, opts, "isPure"))
		.setBinarizer(BinarizerFromString(GetEnumField(env, cls, opts, "binarizer", "Binarizer")))
		.setDownscaleThreshold(GetIntField(env, cls, opts, "downscaleThreshold"))
		.setDownscaleFactor(GetIntField(env, cls, opts, "downscaleFactor"))
		.setMinLineCount(GetIntField(env, cls, opts, "minLineCount"))
		.setMaxNumberOfSymbols(GetIntField(env, cls, opts, "maxNumberOfSymbols"))
		.setTryCode39ExtendedMode(GetBooleanField(env, cls, opts, "tryCode39ExtendedMode"))
		.setReturnErrors(GetBooleanField(env, cls, opts, "returnErrors"))
		.setEanAddOnSymbol(EanAddOnSymbolFromString(GetEnumField(env, cls, opts, "eanAddOnSymbol", "EanAddOnSymbol")))
		.setTextMode(TextModeFromString(GetEnumField(env, cls, opts, "textMode", "TextMode")))
		;
}

extern "C" JNIEXPORT jobject JNICALL
Java_zxingcpp_BarcodeReader_readYBuffer(
	JNIEnv *env, jobject thiz, jobject yBuffer, jint rowStride,
	jint left, jint top, jint width, jint height, jint rotation, jobject options)
{
	const uint8_t* pixels = static_cast<uint8_t *>(env->GetDirectBufferAddress(yBuffer));

	auto image =
		ImageView{pixels + top * rowStride + left, width, height, ImageFormat::Lum, rowStride}
			.rotated(rotation);

	return Read(env, thiz, image, CreateReaderOptions(env, options));
}

struct LockedPixels
{
	JNIEnv* env;
	jobject bitmap;
	void *pixels = nullptr;

	LockedPixels(JNIEnv* env, jobject bitmap) : env(env), bitmap(bitmap) {
		if (AndroidBitmap_lockPixels(env, bitmap, &pixels) != ANDROID_BITMAP_RESUT_SUCCESS)
			pixels = nullptr;
	}

	operator const uint8_t*() const { return static_cast<const uint8_t*>(pixels); }

	~LockedPixels() {
		if (pixels)
			AndroidBitmap_unlockPixels(env, bitmap);
	}
};

extern "C" JNIEXPORT jobject JNICALL
Java_zxingcpp_BarcodeReader_readBitmap(
	JNIEnv* env, jobject thiz, jobject bitmap,
	jint left, jint top, jint width, jint height, jint rotation, jobject options)
{
	AndroidBitmapInfo bmInfo;
	AndroidBitmap_getInfo(env, bitmap, &bmInfo);

	ImageFormat fmt = ImageFormat::None;
	switch (bmInfo.format) {
	case ANDROID_BITMAP_FORMAT_A_8: fmt = ImageFormat::Lum; break;
	case ANDROID_BITMAP_FORMAT_RGBA_8888: fmt = ImageFormat::RGBA; break;
	default: return ThrowJavaException(env, "Unsupported image format in AndroidBitmap");
	}

	auto pixels = LockedPixels(env, bitmap);

	if (!pixels)
		return ThrowJavaException(env, "Failed to lock/read AndroidBitmap data");

	auto image =
		ImageView{pixels, (int)bmInfo.width, (int)bmInfo.height, fmt, (int)bmInfo.stride}
			.cropped(left, top, width, height)
			.rotated(rotation);

	return Read(env, thiz, image, CreateReaderOptions(env, options));
}

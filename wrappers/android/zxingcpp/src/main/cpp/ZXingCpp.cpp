/*
* Copyright 2021 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "JNIUtils.h"
#include "ReadBarcode.h"

#include <android/bitmap.h>
#include <chrono>
#include <exception>
#include <stdexcept>

using namespace ZXing;
using namespace std::string_literals;

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
	case BarcodeFormat::DataBar: return "DATA_BAR";
	case BarcodeFormat::DataBarExpanded: return "DATA_BAR_EXPANDED";
	case BarcodeFormat::UPCA: return "UPC_A";
	case BarcodeFormat::UPCE: return "UPC_E";
	default: throw std::invalid_argument("Invalid format");
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

static EanAddOnSymbol EanAddOnSymbolFromString(const std::string& name)
{
	if (name == "IGNORE") {
		return EanAddOnSymbol::Ignore;
	} else if (name == "READ") {
		return EanAddOnSymbol::Read;
	} else if (name == "REQUIRE") {
		return EanAddOnSymbol::Require;
	} else {
		throw std::invalid_argument("Invalid eanAddOnSymbol name");
	}
}

static Binarizer BinarizerFromString(const std::string& name)
{
	if (name == "LOCAL_AVERAGE") {
		return Binarizer::LocalAverage;
	} else if (name == "GLOBAL_HISTOGRAM") {
		return Binarizer::GlobalHistogram;
	} else if (name == "FIXED_THRESHOLD") {
		return Binarizer::FixedThreshold;
	} else if (name == "BOOL_CAST") {
		return Binarizer::BoolCast;
	} else {
		throw std::invalid_argument("Invalid binarizer name");
	}
}

static TextMode TextModeFromString(const std::string& name)
{
	if (name == "PLAIN") {
		return TextMode::Plain;
	} else if (name == "ECI") {
		return TextMode::ECI;
	} else if (name == "HRI") {
		return TextMode::HRI;
	} else if (name == "HEX") {
		return TextMode::Hex;
	} else if (name == "ESCAPED") {
		return TextMode::Escaped;
	} else {
		throw std::invalid_argument("Invalid textMode name");
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

static jobject NewPosition(JNIEnv* env, const Position& position)
{
	jclass clsPosition = env->FindClass("com/zxingcpp/ZXingCpp$Position");
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
	auto className = "com/zxingcpp/ZXingCpp$"s + type;
	jclass cls = env->FindClass(className.c_str());
	jfieldID fidCT = env->GetStaticFieldID(cls, value, ("L" + className + ";").c_str());
	return env->GetStaticObjectField(cls, fidCT);
}

static jobject NewError(JNIEnv* env, const Error& error)
{
	jclass cls = env->FindClass("com/zxingcpp/ZXingCpp$Error");
	jmethodID midInit = env->GetMethodID(cls, "<init>", "(Lcom/zxingcpp/ZXingCpp$ErrorType;" "Ljava/lang/String;)V");
	return env->NewObject(cls, midInit, NewEnum(env, JavaErrorTypeName(error.type()), "ErrorType"), C2JString(env, error.msg()));
}

static jobject NewResult(JNIEnv* env, const Result& result, int time)
{
	jclass cls = env->FindClass("com/zxingcpp/ZXingCpp$Result");
	jmethodID midInit = env->GetMethodID(
		cls, "<init>",
		"(Lcom/zxingcpp/ZXingCpp$Format;"
		"[B"
		"Ljava/lang/String;"
		"Lcom/zxingcpp/ZXingCpp$ContentType;"
		"Lcom/zxingcpp/ZXingCpp$Position;"
		"I"
		"Ljava/lang/String;"
		"Ljava/lang/String;"
		"I"
		"I"
		"Ljava/lang/String;"
		"Z"
		"I"
		"Lcom/zxingcpp/ZXingCpp$Error;"
		"I)V");
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
		result.error() ? NewError(env, result.error()) : nullptr,
		time
	);
}

static jobject Read(JNIEnv *env, ImageView image, const DecodeHints& hints)
{
	try {
		auto startTime = std::chrono::high_resolution_clock::now();
		auto results = ReadBarcodes(image, hints);
		auto duration = std::chrono::high_resolution_clock::now() - startTime;
//		LOGD("time: %4d ms\n", (int)std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

		jclass clsList = env->FindClass("java/util/ArrayList");
		jobject objList = env->NewObject(clsList, env->GetMethodID(clsList, "<init>", "()V"));
		if (!results.empty()) {
			jmethodID midAdd = env->GetMethodID(clsList, "add", "(Ljava/lang/Object;)Z");
			for (const auto& result: results)
				env->CallBooleanMethod(objList, midAdd, NewResult(env, result, time));
		}
		return objList;
	} catch (const std::exception& e) {
		return ThrowJavaException(env, e.what());
	} catch (...) {
		return ThrowJavaException(env, "Unknown exception");
	}
}

static bool GetBooleanField(JNIEnv* env, jclass cls, jobject hints, const char* name)
{
	return env->GetBooleanField(hints, env->GetFieldID(cls, name, "Z"));
}

static int GetIntField(JNIEnv* env, jclass cls, jobject hints, const char* name)
{
	return env->GetIntField(hints, env->GetFieldID(cls, name, "I"));
}

static std::string GetEnumField(JNIEnv* env, jclass cls, jobject hints, const char* name, const char* type)
{
	auto className = "com/zxingcpp/ZXingCpp$"s + type;
	jmethodID midName = env->GetMethodID(env->FindClass(className.c_str()), "name", "()Ljava/lang/String;");
	jobject objField = env->GetObjectField(hints, env->GetFieldID(cls, name, ("L"s + className + ";").c_str()));
	return J2CString(env, static_cast<jstring>(env->CallObjectMethod(objField, midName)));
}

static BarcodeFormats GetFormats(JNIEnv* env, jclass hintClass, jobject hints)
{
	jobject objField = env->GetObjectField(hints, env->GetFieldID(hintClass, "formats", "Ljava/util/Set;"));
	jmethodID midToArray = env->GetMethodID(env->FindClass("java/util/Set"), "toArray", "()[Ljava/lang/Object;");
	auto objArray = static_cast<jobjectArray>(env->CallObjectMethod(objField, midToArray));
	if (!objArray)
		return {};

	jmethodID midName = env->GetMethodID(env->FindClass("com/zxingcpp/ZXingCpp$Format"), "name", "()Ljava/lang/String;");
	BarcodeFormats ret;
	for (int i = 0, size = env->GetArrayLength(objArray); i < size; ++i) {
		auto objName = static_cast<jstring>(env->CallObjectMethod(env->GetObjectArrayElement(objArray, i), midName));
		ret |= BarcodeFormatFromString(J2CString(env, objName));
	}
	return ret;
}

static DecodeHints CreateDecodeHints(JNIEnv* env, jobject hints)
{
	jclass cls = env->GetObjectClass(hints);
	return DecodeHints()
		.setFormats(GetFormats(env, cls, hints))
		.setTryHarder(GetBooleanField(env, cls, hints, "tryHarder"))
		.setTryRotate(GetBooleanField(env, cls, hints, "tryRotate"))
		.setTryInvert(GetBooleanField(env, cls, hints, "tryInvert"))
		.setTryDownscale(GetBooleanField(env, cls, hints, "tryDownscale"))
		.setIsPure(GetBooleanField(env, cls, hints, "isPure"))
		.setBinarizer(BinarizerFromString(GetEnumField(env, cls, hints, "binarizer", "Binarizer")))
		.setDownscaleThreshold(GetIntField(env, cls, hints, "downscaleThreshold"))
		.setDownscaleFactor(GetIntField(env, cls, hints, "downscaleFactor"))
		.setMinLineCount(GetIntField(env, cls, hints, "minLineCount"))
		.setMaxNumberOfSymbols(GetIntField(env, cls, hints, "maxNumberOfSymbols"))
		.setTryCode39ExtendedMode(GetBooleanField(env, cls, hints, "tryCode39ExtendedMode"))
		.setValidateCode39CheckSum(GetBooleanField(env, cls, hints, "validateCode39CheckSum"))
		.setValidateITFCheckSum(GetBooleanField(env, cls, hints, "validateITFCheckSum"))
		.setReturnCodabarStartEnd(GetBooleanField(env, cls, hints, "returnCodabarStartEnd"))
		.setReturnErrors(GetBooleanField(env, cls, hints, "returnErrors"))
		.setEanAddOnSymbol(EanAddOnSymbolFromString(GetEnumField(env, cls, hints, "eanAddOnSymbol", "EanAddOnSymbol")))
		.setTextMode(TextModeFromString(GetEnumField(env, cls, hints, "textMode", "TextMode")))
		;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_zxingcpp_ZXingCpp_readYBuffer(
	JNIEnv *env, jobject thiz, jobject yBuffer, jint rowStride,
	jint left, jint top, jint width, jint height, jint rotation, jobject hints)
{
	const uint8_t* pixels = static_cast<uint8_t *>(env->GetDirectBufferAddress(yBuffer));

	auto image =
		ImageView{pixels + top * rowStride + left, width, height, ImageFormat::Lum, rowStride}
			.rotated(rotation);

	return Read(env, image, CreateDecodeHints(env, hints));
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
Java_com_zxingcpp_ZXingCpp_readBitmap(
	JNIEnv* env, jobject thiz, jobject bitmap,
	jint left, jint top, jint width, jint height, jint rotation, jobject hints)
{
	AndroidBitmapInfo bmInfo;
	AndroidBitmap_getInfo(env, bitmap, &bmInfo);

	ImageFormat fmt = ImageFormat::None;
	switch (bmInfo.format) {
	case ANDROID_BITMAP_FORMAT_A_8: fmt = ImageFormat::Lum; break;
	case ANDROID_BITMAP_FORMAT_RGBA_8888: fmt = ImageFormat::RGBX; break;
	default: return ThrowJavaException(env, "Unsupported format");
	}

	auto pixels = LockedPixels(env, bitmap);

	if (!pixels)
		return ThrowJavaException(env, "Failed to lock/Read AndroidBitmap data");

	auto image =
		ImageView{pixels, (int)bmInfo.width, (int)bmInfo.height, fmt, (int)bmInfo.stride}
			.cropped(left, top, width, height)
			.rotated(rotation);

	return Read(env, image, CreateDecodeHints(env, hints));
}

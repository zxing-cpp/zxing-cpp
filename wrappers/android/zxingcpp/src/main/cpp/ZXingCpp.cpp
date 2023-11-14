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
	jclass jcls = env->FindClass("java/lang/RuntimeException");
	env->ThrowNew(jcls, message);
	return nullptr;
}

static jobject CreateAndroidPoint(JNIEnv* env, const PointT<int>& point)
{
	jclass cls = env->FindClass("android/graphics/Point");
	auto constructor = env->GetMethodID(cls, "<init>", "(II)V");
	return env->NewObject(cls, constructor, point.x, point.y);
}

static jobject CreatePosition(JNIEnv* env, const Position& position)
{
	jclass cls = env->FindClass("com/zxingcpp/ZXingCpp$Position");
	auto constructor = env->GetMethodID(
		cls, "<init>",
		"(Landroid/graphics/Point;"
		"Landroid/graphics/Point;"
		"Landroid/graphics/Point;"
		"Landroid/graphics/Point;"
		"D)V");
	return env->NewObject(
		cls, constructor,
		CreateAndroidPoint(env, position.topLeft()),
		CreateAndroidPoint(env, position.topRight()),
		CreateAndroidPoint(env, position.bottomLeft()),
		CreateAndroidPoint(env, position.bottomRight()),
		position.orientation());
}

static jbyteArray CreateByteArray(JNIEnv* env, const void* data,
	unsigned int length)
{
	auto size = static_cast<jsize>(length);
	jbyteArray byteArray = env->NewByteArray(size);
	env->SetByteArrayRegion(
		byteArray, 0, size, reinterpret_cast<const jbyte*>(data));
	return byteArray;
}

static jbyteArray CreateByteArray(JNIEnv* env,
	const std::vector<uint8_t>& byteArray)
{
	return CreateByteArray(env,
		reinterpret_cast<const void*>(byteArray.data()),
		byteArray.size());
}

static jobject CreateEnum(JNIEnv* env, const char* enumClass,
	const char* value)
{
	jclass cls = env->FindClass(enumClass);
	jfieldID fidCT = env->GetStaticFieldID(cls, value,
		("L" + std::string(enumClass) + ";").c_str());
	return env->GetStaticObjectField(cls, fidCT);
}

static jobject CreateErrorType(JNIEnv* env, Error::Type errorType)
{
	return CreateEnum(env,
		"com/zxingcpp/ZXingCpp$ErrorType",
		JavaErrorTypeName(errorType));
}

static jobject CreateError(JNIEnv* env, const Error& error)
{
	jclass cls = env->FindClass("com/zxingcpp/ZXingCpp$Error");
	auto constructor = env->GetMethodID(
		cls, "<init>",
		"(Lcom/zxingcpp/ZXingCpp$ErrorType;"
		"Ljava/lang/String;)V");
	return env->NewObject(
		cls, constructor,
		CreateErrorType(env, error.type()),
		C2JString(env, error.msg()));
}

static jobject CreateContentType(JNIEnv* env, ContentType contentType)
{
	return CreateEnum(env,
		"com/zxingcpp/ZXingCpp$ContentType",
		JavaContentTypeName(contentType));
}

static jobject CreateFormat(JNIEnv* env, BarcodeFormat format)
{
	return CreateEnum(env,
		"com/zxingcpp/ZXingCpp$Format",
		JavaBarcodeFormatName(format));
}

static jobject CreateResult(JNIEnv* env, const Result& result,
	const jstring& timeString)
{
	jclass cls = env->FindClass(
		"com/zxingcpp/ZXingCpp$Result");
	auto constructor = env->GetMethodID(
		cls, "<init>",
		"(Lcom/zxingcpp/ZXingCpp$Format;"
		"[B"
		"Ljava/lang/String;"
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
		"Lcom/zxingcpp/ZXingCpp$Error;)V");
	bool valid = result.isValid();
	return env->NewObject(
		cls, constructor,
		CreateFormat(env, result.format()),
		valid ? CreateByteArray(env, result.bytes()) : nullptr,
		valid ? C2JString(env, result.text()) : nullptr,
		timeString,
		CreateContentType(env, result.contentType()),
		CreatePosition(env, result.position()),
		result.orientation(),
		valid ? C2JString(env, result.ecLevel()) : nullptr,
		valid ? C2JString(env, result.symbologyIdentifier()) : nullptr,
		result.sequenceSize(),
		result.sequenceIndex(),
		valid ? C2JString(env, result.sequenceId()) : nullptr,
		result.readerInit(),
		result.lineCount(),
		result.error() ? CreateError(env, result.error()) : nullptr);
}

static jobject Read(JNIEnv *env, ImageView image, const DecodeHints& hints)
{
	try {
		auto startTime = std::chrono::high_resolution_clock::now();
		auto results = ReadBarcodes(image, hints);
		auto duration = std::chrono::high_resolution_clock::now() - startTime;
//		LOGD("time: %4d ms\n", (int)std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
		auto time = std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
		auto timeString = C2JString(env, time);

		auto cls = env->FindClass("java/util/ArrayList");
		auto list = env->NewObject(cls,
			env->GetMethodID(cls, "<init>", "()V"));
		if (!results.empty()) {
			auto add = env->GetMethodID(cls, "add", "(Ljava/lang/Object;)Z");
			for (const auto& result: results) {
				env->CallBooleanMethod(list, add, CreateResult(env, result, timeString));
			}
		}
		return list;
	} catch (const std::exception& e) {
		return ThrowJavaException(env, e.what());
	} catch (...) {
		return ThrowJavaException(env, "Unknown exception");
	}
}

static bool GetBooleanField(JNIEnv* env, jclass cls, jobject hints,
	const char* name)
{
	return env->GetBooleanField(hints, env->GetFieldID(cls, name, "Z"));
}

static int GetIntField(JNIEnv* env, jclass cls, jobject hints,
	const char* name)
{
	return env->GetIntField(hints, env->GetFieldID(cls, name, "I"));
}

static std::string GetEnumField(JNIEnv* env, jclass hintClass, jobject hints,
	const char* enumClass, const char* name)
{
	jclass cls = env->FindClass(enumClass);
	jstring s = (jstring) env->CallObjectMethod(
		env->GetObjectField(hints, env->GetFieldID(hintClass, name,
			("L" + std::string(enumClass) + ";").c_str())),
		env->GetMethodID(cls, "name", "()Ljava/lang/String;"));
	return J2CString(env, s);
}

static std::string JoinFormats(JNIEnv* env, jclass hintClass, jobject hints)
{
	const char* setClass = "java/util/Set";
	jclass cls = env->FindClass(setClass);
	jstring jStr = (jstring) env->CallObjectMethod(
		env->GetObjectField(hints, env->GetFieldID(hintClass, "formats",
			("L" + std::string(setClass) + ";").c_str())),
		env->GetMethodID(cls, "toString", "()Ljava/lang/String;"));
	std::string s = J2CString(env, jStr);
	s.erase(0, s.find_first_not_of("["));
	s.erase(s.find_last_not_of("]") + 1);
	return s;
}

static DecodeHints CreateDecodeHints(JNIEnv* env, jobject hints)
{
	jclass cls = env->GetObjectClass(hints);
	return DecodeHints()
		.setFormats(BarcodeFormatsFromString(JoinFormats(env, cls, hints)))
		.setTryHarder(GetBooleanField(env, cls, hints, "tryHarder"))
		.setTryRotate(GetBooleanField(env, cls, hints, "tryRotate"))
		.setTryInvert(GetBooleanField(env, cls, hints, "tryInvert"))
		.setTryDownscale(GetBooleanField(env, cls, hints, "tryDownscale"))
		.setIsPure(GetBooleanField(env, cls, hints, "isPure"))
		.setTryCode39ExtendedMode(GetBooleanField(env, cls, hints, "tryCode39ExtendedMode"))
		.setValidateCode39CheckSum(GetBooleanField(env, cls, hints, "validateCode39CheckSum"))
		.setValidateITFCheckSum(GetBooleanField(env, cls, hints, "validateITFCheckSum"))
		.setReturnCodabarStartEnd(GetBooleanField(env, cls, hints, "returnCodabarStartEnd"))
		.setReturnErrors(GetBooleanField(env, cls, hints, "returnErrors"))
		.setDownscaleFactor(GetIntField(env, cls, hints, "downscaleFactor"))
		.setEanAddOnSymbol(EanAddOnSymbolFromString(GetEnumField(env, cls, hints,
			"com/zxingcpp/ZXingCpp$EanAddOnSymbol",
			"eanAddOnSymbol")))
		.setBinarizer(BinarizerFromString(GetEnumField(env, cls, hints,
			"com/zxingcpp/ZXingCpp$Binarizer",
			"binarizer")))
		.setTextMode(TextModeFromString(GetEnumField(env, cls, hints,
			"com/zxingcpp/ZXingCpp$TextMode",
			"textMode")))
		.setMinLineCount(GetIntField(env, cls, hints, "minLineCount"))
		.setMaxNumberOfSymbols(GetIntField(env, cls, hints, "maxNumberOfSymbols"))
		.setDownscaleThreshold(GetIntField(env, cls, hints, "downscaleThreshold"));
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_zxingcpp_ZXingCpp_readYBuffer(
	JNIEnv *env, jobject thiz, jobject yBuffer, jint rowStride,
	jint left, jint top, jint width, jint height, jint rotation,
	jobject hints)
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
	jint left, jint top, jint width, jint height, jint rotation,
	jobject hints)
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

	auto image = ImageView{pixels, (int)bmInfo.width, (int)bmInfo.height, fmt, (int)bmInfo.stride}
		.cropped(left, top, width, height)
		.rotated(rotation);

	return Read(env, image, CreateDecodeHints(env, hints));
}

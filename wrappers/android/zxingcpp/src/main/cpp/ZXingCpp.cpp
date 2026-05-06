/*
* Copyright 2021 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ReadBarcode.h"

#include <android/bitmap.h>
#include <android/log.h>
#include <chrono>
#include <exception>
#include <map>
#include <mutex>
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

static jobject NewFormat(JNIEnv* env, BarcodeFormat format)
{
	static std::map<BarcodeFormat, jobject> cache;
	static std::mutex mutex;
	std::scoped_lock<std::mutex> lock(mutex);
	if (cache.empty()) {
		jclass cls = env->FindClass(PACKAGE "Format");
		jmethodID midValues = env->GetStaticMethodID(cls, "values", "()[L" PACKAGE "Format;");
		auto values = static_cast<jobjectArray>(env->CallStaticObjectMethod(cls, midValues));
		jmethodID midGetValue = env->GetMethodID(cls, "getValue", "()I");
		for (int i = 0, size = env->GetArrayLength(values); i < size; ++i) {
			jobject obj = env->GetObjectArrayElement(values, i);
			cache[BarcodeFormat(env->CallIntMethod(obj, midGetValue))] = env->NewGlobalRef(obj);
		}
	}
	if (cache.count(format) == 0)
		throw std::invalid_argument("Invalid BarcodeFormat");
	return cache[format];
}

static jobject NewEnum(JNIEnv* env, int value, const char* type)
{
	auto className = std::string(PACKAGE) + type;
	jclass cls = env->FindClass(className.c_str());
	jmethodID midValues = env->GetStaticMethodID(cls, "values", ("()[L" + className + ";").c_str());
	auto values = static_cast<jobjectArray>(env->CallStaticObjectMethod(cls, midValues));
	return env->GetObjectArrayElement(values, value);
}

static jobject NewError(JNIEnv* env, const Error& error)
{
	jclass cls = env->FindClass(PACKAGE "Error");
	jmethodID midInit = env->GetMethodID(cls, "<init>", "(L" PACKAGE "ErrorType;" "Ljava/lang/String;)V");
	return env->NewObject(cls, midInit, NewEnum(env, static_cast<int>(error.type()), "ErrorType"), C2JString(env, error.msg()));
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
		NewFormat(env, result.format()),
		valid ? NewByteArray(env, result.bytes()) : nullptr,
		valid ? C2JString(env, result.text()) : nullptr,
		NewEnum(env, static_cast<int>(result.contentType()), "ContentType"),
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

static int GetEnumOrdinal(JNIEnv* env, jobject obj)
{
	return env->CallIntMethod(obj, env->GetMethodID(env->GetObjectClass(obj), "ordinal", "()I"));
}

static BarcodeFormats GetFormats(JNIEnv* env, jclass clsOptions, jobject opts)
{
	jobject objField = env->GetObjectField(opts, env->GetFieldID(clsOptions, "formats", "Ljava/util/Set;"));
	jmethodID midToArray = env->GetMethodID(env->FindClass("java/util/Set"), "toArray", "()[Ljava/lang/Object;");
	auto objArray = static_cast<jobjectArray>(env->CallObjectMethod(objField, midToArray));
	if (!objArray)
		return {};

	std::vector<BarcodeFormat> ret;
	for (int i = 0, size = env->GetArrayLength(objArray); i < size; ++i) {
		jobject objFormat = env->GetObjectArrayElement(objArray, i);
		ret.push_back(static_cast<BarcodeFormat>(env->CallIntMethod(objFormat, env->GetMethodID(env->GetObjectClass(objFormat), "getValue", "()I"))));
	}
	return ret;
}

static ReaderOptions CreateReaderOptions(JNIEnv* env, jobject opts)
{
	jclass cls = env->GetObjectClass(opts);
	return ReaderOptions()
		.formats(GetFormats(env, cls, opts))
		.tryHarder(GetBooleanField(env, cls, opts, "tryHarder"))
		.tryRotate(GetBooleanField(env, cls, opts, "tryRotate"))
		.tryInvert(GetBooleanField(env, cls, opts, "tryInvert"))
		.tryDownscale(GetBooleanField(env, cls, opts, "tryDownscale"))
		.tryDenoise(GetBooleanField(env, cls, opts, "tryDenoise"))
		.isPure(GetBooleanField(env, cls, opts, "isPure"))
		.binarizer(static_cast<Binarizer>(GetEnumOrdinal(env, env->GetObjectField(opts, env->GetFieldID(cls, "binarizer", "L" PACKAGE "Binarizer;")))))
		.downscaleThreshold(GetIntField(env, cls, opts, "downscaleThreshold"))
		.downscaleFactor(GetIntField(env, cls, opts, "downscaleFactor"))
		.minLineCount(GetIntField(env, cls, opts, "minLineCount"))
		.maxNumberOfSymbols(GetIntField(env, cls, opts, "maxNumberOfSymbols"))
		.validateOptionalChecksum(GetBooleanField(env, cls, opts, "validateOptionalChecksum"))
		.returnErrors(GetBooleanField(env, cls, opts, "returnErrors"))
		.eanAddOnSymbol(static_cast<EanAddOnSymbol>(GetEnumOrdinal(env, env->GetObjectField(opts, env->GetFieldID(cls, "eanAddOnSymbol", "L" PACKAGE "EanAddOnSymbol;")))))
		.textMode(static_cast<TextMode>(GetEnumOrdinal(env, env->GetObjectField(opts, env->GetFieldID(cls, "textMode", "L" PACKAGE "TextMode;")))))
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

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

static jstring ThrowJavaException(JNIEnv* env, const char* message)
{
	//	if (env->ExceptionCheck())
	//		return 0;
	jclass jcls = env->FindClass("java/lang/RuntimeException");
	env->ThrowNew(jcls, message);
	return nullptr;
}

static jobject CreateContentType(JNIEnv* env, ContentType contentType)
{
	jclass cls = env->FindClass("com/zxingcpp/BarcodeReader$ContentType");
	jfieldID fidCT = env->GetStaticFieldID(cls , JavaContentTypeName(contentType), "Lcom/zxingcpp/BarcodeReader$ContentType;");
	return env->GetStaticObjectField(cls, fidCT);
}

static jobject CreateAndroidPoint(JNIEnv* env, const PointT<int>& point)
{
	jclass cls = env->FindClass("android/graphics/Point");
	auto constructor = env->GetMethodID(cls, "<init>", "(II)V");
	return env->NewObject(cls, constructor, point.x, point.y);
}

static jobject CreatePosition(JNIEnv* env, const Position& position)
{
	jclass cls = env->FindClass("com/zxingcpp/BarcodeReader$Position");
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

jstring Read(JNIEnv *env, ImageView image, jstring formats, jboolean tryHarder, jboolean tryRotate,
			 jboolean tryDownscale, jobject result)
{
	try {
		auto hints = DecodeHints()
						 .setFormats(BarcodeFormatsFromString(J2CString(env, formats)))
						 .setTryHarder(tryHarder)
						 .setTryRotate( tryRotate )
						 .setTryDownscale(tryDownscale)
						 .setMaxNumberOfSymbols(1);

		auto startTime = std::chrono::high_resolution_clock::now();
		auto results = ReadBarcodes(image, hints);
		auto duration = std::chrono::high_resolution_clock::now() - startTime;
//		LOGD("time: %4d ms\n", (int)std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());

		jclass clResult = env->GetObjectClass(result);

		jfieldID fidTime = env->GetFieldID(clResult, "time", "Ljava/lang/String;");
		auto time = std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
		env->SetObjectField(result, fidTime, C2JString(env, time));

		if (!results.empty()) {
			auto& res = results.front();
			jbyteArray jByteArray = env->NewByteArray(res.bytes().size());
			env->SetByteArrayRegion(jByteArray, 0, res.bytes().size(), (jbyte*)res.bytes().data());
			jfieldID fidBytes = env->GetFieldID(clResult, "bytes", "[B");
			env->SetObjectField(result, fidBytes, jByteArray);

			jfieldID fidText = env->GetFieldID(clResult, "text", "Ljava/lang/String;");
			env->SetObjectField(result, fidText, C2JString(env, res.text()));

			jfieldID fidContentType = env->GetFieldID(clResult , "contentType", "Lcom/zxingcpp/BarcodeReader$ContentType;");
			env->SetObjectField(result, fidContentType, CreateContentType(env, res.contentType()));

			jfieldID fidPosition = env->GetFieldID(clResult, "position", "Lcom/zxingcpp/BarcodeReader$Position;");
			env->SetObjectField(result, fidPosition, CreatePosition(env, res.position()));

			jfieldID fidOrientation = env->GetFieldID(clResult, "orientation", "I");
			env->SetIntField(result, fidOrientation, res.orientation());

			jfieldID fidEcLevel = env->GetFieldID(clResult, "ecLevel", "Ljava/lang/String;");
			env->SetObjectField(result, fidEcLevel, C2JString(env, res.ecLevel()));

			jfieldID fidSymbologyIdentifier = env->GetFieldID(clResult, "symbologyIdentifier", "Ljava/lang/String;");
			env->SetObjectField(result, fidSymbologyIdentifier, C2JString(env, res.symbologyIdentifier()));

			return C2JString(env, JavaBarcodeFormatName(res.format()));
		} else
			return C2JString(env, "NotFound");
	} catch (const std::exception& e) {
		return ThrowJavaException(env, e.what());
	} catch (...) {
		return ThrowJavaException(env, "Unknown exception");
	}
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_zxingcpp_BarcodeReader_readYBuffer(
	JNIEnv *env, jobject thiz, jobject yBuffer, jint rowStride,
	jint left, jint top, jint width, jint height, jint rotation,
	jstring formats, jboolean tryHarder, jboolean tryRotate, jboolean tryDownscale,
	jobject result)
{
	const uint8_t* pixels = static_cast<uint8_t *>(env->GetDirectBufferAddress(yBuffer));

	auto image =
		ImageView{pixels + top * rowStride + left, width, height, ImageFormat::Lum, rowStride}
			.rotated(rotation);

	return Read(env, image, formats, tryHarder, tryRotate, tryDownscale, result);
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

extern "C" JNIEXPORT jstring JNICALL
Java_com_zxingcpp_BarcodeReader_readBitmap(
	JNIEnv* env, jobject thiz, jobject bitmap,
	jint left, jint top, jint width, jint height, jint rotation,
	jstring formats, jboolean tryHarder, jboolean tryRotate, jboolean tryDownscale,
	jobject result)
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

	return Read(env, image, formats, tryHarder, tryRotate, tryDownscale, result);
}

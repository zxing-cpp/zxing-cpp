/*
* Copyright 2021 Axel Waggershauser
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "BitMatrix.h"
#include "CharacterSetECI.h"
#include "DecodeStatus.h"
#include "GTIN.h"
#include "JNIUtils.h"
#include "ReadBarcode.h"
#include "MultiFormatWriter.h"
#include "TextUtfEncoding.h"

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
	case BarcodeFormat::DataBar: return "DATA_BAR";
	case BarcodeFormat::DataBarExpanded: return "DATA_BAR_EXPANDED";
	case BarcodeFormat::UPCA: return "UPC_A";
	case BarcodeFormat::UPCE: return "UPC_E";
	default: throw std::invalid_argument("Invalid format");
	}
}

static jobject ThrowJavaException(JNIEnv* env, const char* message)
{
	jclass jcls = env->FindClass("java/lang/RuntimeException");
	env->ThrowNew(jcls, message);
	return nullptr;
}

static jobject CreateBitMatrix(JNIEnv* env, int width, int height, jbyteArray data)
{
	jclass cls = env->FindClass("com/nubook/android/zxingcpp/ZxingCpp$BitMatrix");
	auto constructor = env->GetMethodID(
		cls, "<init>", "(II[B)V");
	return env->NewObject(cls, constructor, width, height, data);
}

static jobject CreateBitMatrix(JNIEnv* env, const Matrix<uint8_t>& bm)
{
	jsize size = bm.size();
	jbyteArray data = env->NewByteArray(size);
	env->SetByteArrayRegion(
		data, 0, size, reinterpret_cast<const jbyte*>(bm.data()));
	return CreateBitMatrix(env, bm.width(), bm.height(), data);
}

static jobject CreateGTIN(JNIEnv* env, const std::string& country,
	const std::string& addOn, const std::string& price,
	const std::string& issueNumber)
{
	jclass cls = env->FindClass("com/nubook/android/zxingcpp/ZxingCpp$GTIN");
	auto constructor = env->GetMethodID(
		cls, "<init>",
		"(Ljava/lang/String;"
			"Ljava/lang/String;"
			"Ljava/lang/String;"
			"Ljava/lang/String;)V");
	return env->NewObject(
		cls, constructor,
		C2JString(env, country),
		C2JString(env, addOn),
		C2JString(env, price),
		C2JString(env, issueNumber));
}

static jobject CreateOptionalGTIN(JNIEnv* env, const Result& result)
{
	if ((BarcodeFormat::EAN13 | BarcodeFormat::EAN8 | BarcodeFormat::UPCA |
		BarcodeFormat::UPCE).testFlag(result.format())) {
		return CreateGTIN(
			env,
			GTIN::LookupCountryIdentifier(
				TextUtfEncoding::ToUtf8(result.text()), result.format()),
			GTIN::EanAddOn(result),
			GTIN::Price(GTIN::EanAddOn(result)),
			GTIN::IssueNr(GTIN::EanAddOn(result)));
	} else if (result.format() == BarcodeFormat::ITF && result.text().length() == 14) {
		return CreateGTIN(
			env,
			GTIN::LookupCountryIdentifier(
				TextUtfEncoding::ToUtf8(result.text()), result.format()),
			"", "", "");
	}
	return nullptr;
}

static jobject CreateAndroidRect(JNIEnv* env, int left, int top, int right, int bottom)
{
	jclass cls = env->FindClass("android/graphics/Rect");
	auto constructor = env->GetMethodID(cls, "<init>", "(IIII)V");
	return env->NewObject(cls, constructor, left, top, right, bottom);
}

static jobject CreateAndroidRect(JNIEnv* env, const Position& position)
{
	auto tl = position.topLeft();
	auto br = position.bottomRight();
	return CreateAndroidRect(env, tl.x, tl.y, br.x, br.y);
}

static jobject CreateResult(JNIEnv* env, const Result& result)
{
	jclass cls = env->FindClass(
		"com/nubook/android/zxingcpp/ZxingCpp$Result");
	auto constructor = env->GetMethodID(
		cls, "<init>",
		"(Ljava/lang/String;"
		"Ljava/lang/String;"
		"Landroid/graphics/Rect;"
		"I"
		"[B"
		"I"
		"Ljava/lang/String;"
		"Ljava/lang/String;"
		"I"
		"I"
		"Ljava/lang/String;"
		"Z"
		"I"
		"Lcom/nubook/android/zxingcpp/ZxingCpp$GTIN;)V");
	const auto& rb = result.rawBytes();
	auto size = static_cast<jsize>(rb.size());
	jbyteArray rawBytes = env->NewByteArray(size);
	env->SetByteArrayRegion(
		rawBytes, 0, size, reinterpret_cast<const jbyte*>(rb.data()));
	jobject gtin = CreateOptionalGTIN(env, result);
	return env->NewObject(
		cls, constructor,
		C2JString(env, JavaBarcodeFormatName(result.format())),
		C2JString(env, result.text()),
		CreateAndroidRect(env, result.position()),
		result.orientation(),
		rawBytes,
		result.numBits(),
		C2JString(env, result.ecLevel()),
		C2JString(env, result.symbologyIdentifier()),
		result.sequenceSize(),
		result.sequenceIndex(),
		C2JString(env, result.sequenceId()),
		result.readerInit(),
		result.lineCount(),
		gtin);
}

static jobject Read(JNIEnv* env, ImageView image, jstring formats,
	jboolean tryHarder, jboolean tryRotate)
{
	try {
		auto hints = DecodeHints()
			.setFormats(BarcodeFormatsFromString(J2CString(env, formats)))
			.setTryHarder(tryHarder)
			.setTryRotate(tryRotate);
		auto res = ReadBarcode(image, hints);
		if (res.isValid()) {
			// Only allocate Result when ReadBarcode() found something.
			return CreateResult(env, res);
		} else if (res.status() == ZXing::DecodeStatus::NotFound){
			// From the perspective of a camera app, it's not an exception when
			// no barcode could be found. It's a perfectly valid state and in no
			// way exceptional.
			return nullptr;
		} else {
			return ThrowJavaException(env, ToString(res.status()));
		}
	} catch (const std::exception& e) {
		return ThrowJavaException(env, e.what());
	} catch (...) {
		return ThrowJavaException(env, "Unknown exception");
	}
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_nubook_android_zxingcpp_ZxingCpp_readYBuffer(
	JNIEnv* env, jobject, jobject yBuffer, jint rowStride,
	jint left, jint top, jint width, jint height, jint rotation,
	jstring formats, jboolean tryHarder, jboolean tryRotate)
{
	const uint8_t* pixels = static_cast<uint8_t*>(
		env->GetDirectBufferAddress(yBuffer));

	auto image = ImageView{
		pixels + top * rowStride + left,
		width,
		height,
		ImageFormat::Lum,
		rowStride
	}.rotated(rotation);

	return Read(env, image, formats, tryHarder, tryRotate);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_nubook_android_zxingcpp_ZxingCpp_readByteArray(
	JNIEnv* env, jobject, jbyteArray yuvData, jint rowStride,
	jint left, jint top, jint width, jint height, jint rotation,
	jstring formats, jboolean tryHarder, jboolean tryRotate)
{
	auto *pixels = env->GetByteArrayElements(yuvData, nullptr);

	auto image = ImageView{
		reinterpret_cast<uint8_t*>(pixels) + top * rowStride + left,
		width,
		height,
		ImageFormat::Lum,
		rowStride
	}.rotated(rotation);

	auto result = Read(env, image, formats, tryHarder, tryRotate);
	env->ReleaseByteArrayElements(yuvData, pixels, 0);

	return result;
}

struct LockedPixels
{
	JNIEnv* env;
	jobject bitmap;
	void* pixels = nullptr;

	LockedPixels(JNIEnv* env, jobject bitmap) : env(env), bitmap(bitmap) {
		if (AndroidBitmap_lockPixels(env, bitmap, &pixels) != ANDROID_BITMAP_RESUT_SUCCESS) {
			pixels = nullptr;
		}
	}

	operator const uint8_t*() const {
		return static_cast<const uint8_t*>(pixels);
	}

	virtual ~LockedPixels() {
		if (pixels) {
			AndroidBitmap_unlockPixels(env, bitmap);
		}
	}
};

extern "C" JNIEXPORT jobject JNICALL
Java_com_nubook_android_zxingcpp_ZxingCpp_readBitmap(
	JNIEnv* env, jobject, jobject bitmap,
	jint left, jint top, jint width, jint height, jint rotation,
	jstring formats, jboolean tryHarder, jboolean tryRotate)
{
	AndroidBitmapInfo bmInfo;
	AndroidBitmap_getInfo(env, bitmap, &bmInfo);

	ImageFormat fmt;
	switch (bmInfo.format) {
	case ANDROID_BITMAP_FORMAT_A_8: fmt = ImageFormat::Lum; break;
	case ANDROID_BITMAP_FORMAT_RGBA_8888: fmt = ImageFormat::RGBX; break;
	default: return ThrowJavaException(env, "Unsupported format");
	}

	auto pixels = LockedPixels(env, bitmap);

	if (!pixels) {
		return ThrowJavaException(env, "Failed to lock/Read AndroidBitmap data");
	}

	auto image = ImageView{
		pixels,
		(int)bmInfo.width,
		(int)bmInfo.height,
		fmt,
		(int)bmInfo.stride
	}.cropped(left, top, width, height).rotated(rotation);

	return Read(env, image, formats, tryHarder, tryRotate);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_nubook_android_zxingcpp_ZxingCpp_encode(
	JNIEnv* env, jobject, jstring text, jstring format,
	jint width, jint height, jint margin, jint eccLevel, jstring encoding)
{
	try {
		auto writer = MultiFormatWriter(BarcodeFormatFromString(J2CString(env, format)))
			.setEncoding(CharacterSetECI::CharsetFromName(J2CString(env, encoding).c_str()))
			.setMargin(margin)
			.setEccLevel(eccLevel);
		auto matrix = ToMatrix<uint8_t>(writer.encode(
			TextUtfEncoding::FromUtf8(J2CString(env, text)), width, height));
		return CreateBitMatrix(env, matrix);
	} catch (const std::exception& e) {
		ThrowJavaException(env, e.what());
		return nullptr;
	}
}

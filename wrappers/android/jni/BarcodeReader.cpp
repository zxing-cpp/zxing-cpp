/*
* Copyright 2016 Nu-book Inc.
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

#include "JNIUtils.h"
#include "MultiFormatReader.h"
#include "DecodeHints.h"
#include "Result.h"

#include <exception>
#include <vector>

// This is what we have at Java's side and must be kept in sync with.
enum class JavaBarcodeFormat : int {
	AZTEC,
	CODABAR,
	CODE_39,
	CODE_93,
	CODE_128,
	DATA_MATRIX,
	EAN_8,
	EAN_13,
	ITF,
	MAXICODE,
	PDF_417,
	QR_CODE,
	RSS_14,
	RSS_EXPANDED,
	UPC_A,
	UPC_E,
	UPC_EAN_EXTENSION
};


static ZXing::BarcodeFormat ToZXingBarcodeFormat(JNIEnv* env, JavaBarcodeFormat format)
{
    switch (format) {
        case JavaBarcodeFormat::AZTEC             : return ZXing::BarcodeFormat::AZTEC;
        case JavaBarcodeFormat::CODABAR           : return ZXing::BarcodeFormat::CODABAR;
        case JavaBarcodeFormat::CODE_39           : return ZXing::BarcodeFormat::CODE_39;
        case JavaBarcodeFormat::CODE_93           : return ZXing::BarcodeFormat::CODE_93;
        case JavaBarcodeFormat::CODE_128          : return ZXing::BarcodeFormat::CODE_128;
        case JavaBarcodeFormat::DATA_MATRIX       : return ZXing::BarcodeFormat::DATA_MATRIX;
        case JavaBarcodeFormat::EAN_8             : return ZXing::BarcodeFormat::EAN_8;
        case JavaBarcodeFormat::EAN_13            : return ZXing::BarcodeFormat::EAN_13;
        case JavaBarcodeFormat::ITF               : return ZXing::BarcodeFormat::ITF;
        case JavaBarcodeFormat::MAXICODE          : return ZXing::BarcodeFormat::MAXICODE;
        case JavaBarcodeFormat::PDF_417           : return ZXing::BarcodeFormat::PDF_417;
        case JavaBarcodeFormat::QR_CODE           : return ZXing::BarcodeFormat::QR_CODE;
        case JavaBarcodeFormat::RSS_14            : return ZXing::BarcodeFormat::RSS_14;
        case JavaBarcodeFormat::RSS_EXPANDED      : return ZXing::BarcodeFormat::RSS_EXPANDED;
        case JavaBarcodeFormat::UPC_A             : return ZXing::BarcodeFormat::UPC_A;
        case JavaBarcodeFormat::UPC_E             : return ZXing::BarcodeFormat::UPC_E;
        case JavaBarcodeFormat::UPC_EAN_EXTENSION : return ZXing::BarcodeFormat::UPC_EAN_EXTENSION;
    }
    ThrowJavaException(env, "Invalid format");
}

static ZXing::BarcodeFormats GetFormats(JNIEnv* env, jintArray formats)
{
	ZXing::BarcodeFormats result = ZXing::BarcodeFormat::NONE;
	jsize len = env->GetArrayLength(formats);
	if (len > 0) {
		std::vector<jint> elems(len);
		env->GetIntArrayRegion(formats, 0, elems.size(), elems.data());
		for (jsize i = 0; i < len; ++i) {
			result |= ToZXingBarcodeFormat(env, static_cast<JavaBarcodeFormat>(elems[i]));
		}
	}
	return result;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_zxing_BarcodeReader_createInstance(JNIEnv* env, jobject thiz, jintArray formats)
{
	try
	{
		ZXing::DecodeHints hints;
		if (formats != nullptr) {
			hints.setFormats(GetFormats(env, formats));
		}
		return reinterpret_cast<jlong>(new ZXing::MultiFormatReader(hints));
	}
	catch (const std::exception& e)
	{
		ThrowJavaException(env, e.what());
	}
	catch (...)
	{
		ThrowJavaException(env, "Unknown exception");
	}
	return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_zxing_BarcodeReader_destroyInstance(JNIEnv* env, jobject thiz, jlong objPtr)
{
	try
	{
		delete reinterpret_cast<ZXing::MultiFormatReader*>(objPtr);
	}
	catch (const std::exception& e)
	{
		ThrowJavaException(env, e.what());
	}
	catch (...)
	{
		ThrowJavaException(env, "Unknown exception");
	}
}

extern "C" JNIEXPORT jint JNICALL
Java_com_zxing_BarcodeReader_readBarcode(JNIEnv* env, jobject thiz, jlong objPtr, jobject bitmap, jint left, jint top, jint width, jint height, jobjectArray result)
{
	try
	{
		auto reader = reinterpret_cast<ZXing::MultiFormatReader*>(objPtr);
		auto binImage = BinaryBitmapFromJavaBitmap(env, bitmap, left, top, width, height);
		auto readResult = reader->read(*binImage);
		if (readResult.isValid()) {
			env->SetObjectArrayElement(result, 0, ToJavaString(env, readResult.text()));
			return static_cast<int>(readResult.format());
		}
	}
	catch (const std::exception& e)
	{
		ThrowJavaException(env, e.what());
	}
	catch (...)
	{
		ThrowJavaException(env, "Unknown exception");
	}
	return -1;
}

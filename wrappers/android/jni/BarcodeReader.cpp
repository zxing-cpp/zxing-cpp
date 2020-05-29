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

static std::vector<ZXing::BarcodeFormat> GetFormats(JNIEnv* env, jintArray formats)
{
	std::vector<ZXing::BarcodeFormat> result;
	jsize len = env->GetArrayLength(formats);
	if (len > 0) {
		std::vector<jint> elems(len);
		env->GetIntArrayRegion(formats, 0, elems.size(), elems.data());
		result.resize(len);
		for (jsize i = 0; i < len; ++i) {
			result[i] = ZXing::BarcodeFormat(elems[i]);
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
			hints.setPossibleFormats(GetFormats(env, formats));
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

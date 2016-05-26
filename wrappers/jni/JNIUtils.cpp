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
#include "GenericLumuninanceSource.h"
#include "HybridBinarizer.h"

#include <android/bitmap.h>
#include <stdexcept>

namespace ZXing {

namespace {

	struct AutoUnlockPixels
	{
		JNIEnv* m_env;
		jobject m_bitmap;
		AutoUnlockPixels(JNIEnv* env, jobject bitmap) :  m_env(env), m_bitmap(bitmap) {}
		
		~AutoUnlockPixels() {
			AndroidBitmap_unlockPixels(m_env, m_bitmap);
		}
	};

} // anonymous

std::shared_ptr<BinaryBitmap> CreateBinaryBitmap(JNIEnv* env, jobject bitmap, int cropWidth, int cropHeight)
{
	AndroidBitmapInfo bmInfo;
	AndroidBitmap_getInfo(env, bitmap, &bmInfo);

	cropWidth = cropWidth <= 0 ? (int)bmInfo.width : std::min((int)bmInfo.width, cropWidth);
	cropHeight = cropHeight <= 0 ? (int)bmInfo.height : std::min((int)bmInfo.height, cropHeight);
	int cropLeft = ((int)bmInfo.width - cropWidth) / 2;
	int cropTop = ((int)bmInfo.height - cropHeight) / 2;

	void *pixels = nullptr;
	if (AndroidBitmap_lockPixels(env, bitmap, &pixels) == ANDROID_BITMAP_RESUT_SUCCESS)
	{
		AutoUnlockPixels autounlock(env, bitmap);
		
		std::shared_ptr<GenericLuminanceSource> luminance;
		switch (bmInfo.format)
		{
			case ANDROID_BITMAP_FORMAT_A_8:
				luminance = std::make_shared<GenericLuminanceSource>(cropLeft, cropTop, cropWidth, cropHeight, inBytes, inBuffer->GetPlaneDescription(0).Stride);
				break;
			case ANDROID_BITMAP_FORMAT_RGBA_8888:
				luminance = std::make_shared<GenericLuminanceSource>(cropLeft, cropTop, cropWidth, cropHeight, inBytes, inBuffer->GetPlaneDescription(0).Stride, 4, 0, 1, 2);
				break;
			default:
				throw std::runtime_error("Unsupported format");
		}
		return std::make_shared<HybridBinarizer>(luminance);
	}
	else
	{
		throw std::runtime_error("Failed to read bitmap's data");
	}
}

void ThrowJavaException(JNIEnv* env, const char* message)
{
	static jclass jcls = env->FindClass("java/lang/RuntimeException");
	env->ThrowNew(jcls, message);
}

} // ZXing

#pragma once
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

#include <jni.h>
#include <android/log.h>

#include <memory>
#include <string>

#define ZX_LOG_TAG "ZXing"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, ZX_LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, ZX_LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, ZX_LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, ZX_LOG_TAG, __VA_ARGS__)

namespace ZXing {
class BinaryBitmap;
}

// Create BinaryBitmap from Android's Bitmap
std::shared_ptr<ZXing::BinaryBitmap> BinaryBitmapFromJavaBitmap(JNIEnv* env, jobject bitmap, int cropLeft, int cropTop, int cropWidth, int cropHeight);
void ThrowJavaException(JNIEnv* env, const char* message);
jstring ToJavaString(JNIEnv* env, const std::wstring& str);

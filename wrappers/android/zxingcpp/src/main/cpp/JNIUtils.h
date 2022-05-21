/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <jni.h>
#include <android/log.h>

#include <string>

#define ZX_LOG_TAG "ZXing"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, ZX_LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, ZX_LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, ZX_LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, ZX_LOG_TAG, __VA_ARGS__)

jstring C2JString(JNIEnv* env, const std::wstring& str);
jstring C2JString(JNIEnv* env, const std::string& str);
std::string J2CString(JNIEnv* env, jstring str);

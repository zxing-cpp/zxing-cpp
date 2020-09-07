LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := zxing_android

LOCAL_CFLAGS += -Wall
LOCAL_CPPFLAGS += -std=c++17

LOCAL_SRC_FILES := \
	JNIUtils.cpp \
	BarcodeReader.cpp

LOCAL_CPP_FEATURES += rtti exceptions

LOCAL_STATIC_LIBRARIES := zxing_core
LOCAL_LDLIBS := -llog -ljnigraphics

ifeq ($(TARGET_ARCH_ABI), armeabi)
    LOCAL_LDLIBS += -latomic
endif

include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/../../../core/Android.mk

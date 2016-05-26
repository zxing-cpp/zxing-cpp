LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := zxing_core

LOCAL_CFLAGS += -Wall
LOCAL_CPPFLAGS += -std=c++11

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/src

COMMON_FILES :=		$(wildcard $(LOCAL_PATH)/src/*.cpp)
AZTEC_FILES :=		$(wildcard $(LOCAL_PATH)/src/aztec/*.cpp)
DATAMATRIX_FILES :=	$(wildcard $(LOCAL_PATH)/src/datamatrix/*.cpp)
ONED_FILES :=		$(wildcard $(LOCAL_PATH)/src/oned/*.cpp)
ONED_RSS_FILES :=	$(wildcard $(LOCAL_PATH)/src/oned/rss/*.cpp)
PDF417_FILES :=		$(wildcard $(LOCAL_PATH)/src/pdf417/*.cpp)
QRCODE_FILES :=		$(wildcard $(LOCAL_PATH)/src/qrcode/*.cpp)
TEXT_CODEC_FILES :=	$(wildcard $(LOCAL_PATH)/src/textcodec/*.cpp)

LOCAL_SRC_FILES := \
	$(COMMON_FILES) \
	$(AZTEC_FILES) \
	$(DATAMATRIX_FILES) \
	$(ONED_FILES) \
	$(ONED_RSS_FILES) \
	$(PDF417_FILES) \
	$(QRCODE_FILES) \
	$(TEXT_CODEC_FILES)

LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_LDLIBS := -llog -ljnigraphics

include $(BUILD_STATIC_LIBRARY)

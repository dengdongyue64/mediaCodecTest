LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	avplayer.cpp \
	encode.cpp \
	main.cpp
#APP_STL := c++_static
LOCAL_CLANG := true
#LOCAL_CPPFLAGS := -fpermissive 
LOCAL_MODULE := avtest

LOCAL_MODULE_TAGS :=  optional

LOCAL_SHARED_LIBRARIES := \
	libc \
	libstagefright liblog libutils libbinder libstagefright_foundation \
    libmedia libgui libcutils libui  libz
	
LOCAL_C_INCLUDES := \
	$(TOP)/frameworks/av/media/libstagefright \
	$(TOP)/frameworks/av/media/libstagefright/include \
	$(TOP)/frameworks/native/include/media/openmax 
	
LOCAL_CFLAGS = -std=c++11
LOCAL_CFLAGS += -Wno-multichar -Werror -Wall -Wno-unused-parameter
LOCAL_LDFLAGS := -ldl
LOCAL_32_BIT_ONLY := true
include $(BUILD_EXECUTABLE)

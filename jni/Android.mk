LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -std=c++11 -Wall -Wextra
LOCAL_MODULE := spir2cross
LOCAL_SRC_FILES := ../spir2cross.cpp ../spir2glsl.cpp ../spir2cpp.cpp
LOCAL_CPP_FEATURES := exceptions
LOCAL_ARM_MODE := arm

include $(BUILD_STATIC_LIBRARY)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -std=c++11 -Wall -Wextra
LOCAL_MODULE := spirv-cross
LOCAL_SRC_FILES := ../spirv_cross.cpp ../spirv_glsl.cpp ../spirv_cpp.cpp
LOCAL_CPP_FEATURES := exceptions
LOCAL_ARM_MODE := arm

include $(BUILD_STATIC_LIBRARY)

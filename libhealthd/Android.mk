LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := healthd_board_custom.cpp
LOCAL_MODULE := libhealthd.socleds
LOCAL_C_INCLUDES := system/core/healthd
LOCAL_EXPORT_C_INCLUDE_DIRS := system/core/include
LOCAL_CFLAGS := -Werror
include $(BUILD_STATIC_LIBRARY)

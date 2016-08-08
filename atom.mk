LOCAL_PATH := $(call my-dir)

###############################################################################
###############################################################################

include $(CLEAR_VARS)
LOCAL_MODULE := libmuta
LOCAL_CATEGORY_PATH := libs
LOCAL_DESCRIPTION := Multimedia User Transforming Algorithm

LOCAL_SRC_FILES := src/muta.c
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_LIBRARIES := libulog gstreamer libpomp gst-plugins-base

include $(BUILD_LIBRARY)

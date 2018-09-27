LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := libeeprom_jni
LOCAL_SRC_FILES := eeprom_jni.c
LOCAL_SHARED_LIBRARIES :=  libutils libcutils
include $(BUILD_SHARED_LIBRARY)

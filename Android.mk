LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/$(SDL_PATH)/include \
	$(LOCAL_PATH)/../SDL_image \
	$(LOCAL_PATH)/../SDL_mixer \
	$(LOCAL_PATH)/../SDL_ttf \
	$(LOCAL_PATH)/../SDL_net \
	$(LOCAL_PATH)/../SDL_gfx \

# Add any compilation flags for your project here...
LOCAL_CFLAGS := \
	-DPLAY_MOD

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.cpp \
	battle0-client.c \
	network.c
	

LOCAL_SHARED_LIBRARIES := SDL SDL_image SDL_mixer SDL_ttf SDL_net SDL_gfx

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

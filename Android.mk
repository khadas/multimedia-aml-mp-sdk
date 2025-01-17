LOCAL_PATH:= $(call my-dir)

#######################################
AML_REFERENCE_PATH := $(TOP)/vendor/amlogic/common
ifeq (1, $(shell expr $(PLATFORM_SDK_VERSION) \>= 31))
AML_REFERENCE_PATH := $(TOP)/vendor/amlogic/reference
endif

ifeq ($(BUILD_WITH_WIDEVINECAS),true)
ifneq (, $(wildcard $(TOP)/vendor/amlogic/common/prebuilt/libmediadrm/wvcas/include))
HAVE_WVIPTV_CAS := true
endif
endif

ifneq (, $(wildcard $(AML_REFERENCE_PATH)/external/DTVKit/cas_hal))
HAVE_CAS_HAL := true
endif

ifeq (1, $(shell expr $(PLATFORM_SDK_VERSION) \>= 30))
HAVE_VMXIPTV_CAS := true
HAVE_VMXWEB_CAS := true
endif

HAVE_NAGRAWEB_CAS := false

ifeq (1, $(shell expr $(PLATFORM_SDK_VERSION) \>= 31))
HAVE_TUNER_HAL := true
endif

#######################################
AML_MP_PLAYER_SRC := \
	player/Aml_MP.cpp \
	player/Aml_MP_Player.cpp \
	player/Aml_MP_PlayerImpl.cpp \
	player/AmlPlayerBase.cpp \
	player/AmlTsPlayer.cpp \
	player/AmlCTCPlayer.cpp \
	player/AmlDummyTsPlayer.cpp \
	mediaplayer/Aml_MP_MediaPlayer.cpp \
	mediaplayer/Aml_MP_MediaPlayerImpl.cpp \
	mediaplayer/AmlMediaPlayerBase.cpp \

#AML_MP_PLAYER_SYSTEM_SRC_29 := \
	player/AmlCTCPlayer.cpp \

#AML_MP_PLAYER_VENDOR_SRC_29 := \
	player/AmlCTCPlayer.cpp \

ifeq ($(HAVE_TUNER_HAL), true)
AML_MP_PLAYER_SYSTEM_SRC_ge_30 += \
	player/AmlTvPlayer.cpp
endif

AML_MP_CAS_SRC := \
	cas/Aml_MP_CAS.cpp \
	cas/AmlCasBase.cpp \
	cas/AmlDvbCasHal.cpp \
	cas/AmCasLibWrapper.cpp \

#AML_MP_CAS_SYSTEM_SRC_29 += \
	cas/vmx_iptvcas/AmlVMXIptvCas.cpp

# AML_MP_CAS_VENDOR_SRC_ge_30 += \
	cas/vmx_iptvcas/AmlVMXIptvCas_V2.cpp

ifeq ($(HAVE_WVIPTV_CAS), true)
AML_MP_CAS_VENDOR_SRC_ge_30 += \
	cas/wv_iptvcas/AmlWVIptvCas_V2.cpp
AML_MP_CAS_SYSTEM_SRC_ge_30 += \
	cas/wv_iptvcas/AmlWVIptvCas_V2.cpp
endif

ifeq ($(HAVE_VMXIPTV_CAS), true)
AML_MP_CAS_SYSTEM_SRC_ge_30 += \
	cas/vmx_iptvcas/AmlVMXIptvCas_V2.cpp
endif

ifeq ($(HAVE_VMXWEB_CAS), true)
AML_MP_CAS_SYSTEM_SRC_ge_30 += \
	cas/vmx_webcas/AmlVMXWebCas.cpp
endif

ifeq ($(HAVE_NAGRAWEB_CAS), true)
AML_MP_CAS_VENDOR_SRC_ge_30 += \
	cas/nagra_webcas/AmlNagraWebCas.cpp
AML_MP_CAS_SYSTEM_SRC_ge_30 += \
	cas/nagra_webcas/AmlNagraWebCas.cpp
endif

AML_MP_DVR_SRC := \
	dvr/Aml_MP_DVR.cpp \
	dvr/AmlDVRPlayer.cpp \
	dvr/AmlDVRRecorder.cpp

AML_MP_DEMUX_SRC := \
	demux/AmlDemuxBase.cpp \
	demux/AmlHwDemux.cpp \
	demux/AmlSwDemux.cpp \
	demux/AmlESQueue.cpp \
	demux/AmlTsParser.cpp

ifeq ($(HAVE_TUNER_HAL), true)
AML_MP_DEMUX_SYSTEM_SRC_ge_30 += \
	tunerhal/TunerService.cpp \
	tunerhal/TunerDemux.cpp \
	tunerhal/TunerDvr.cpp \
	tunerhal/TunerFilter.cpp \
	tunerhal/MediaCodecWrapper.cpp \
	tunerhal/AudioTrackWrapper.cpp \
	demux/AmlTunerHalDemux.cpp
endif

AML_MP_UTILS_SRC := \
	utils/AmlMpAtomizer.cpp \
	utils/AmlMpBitReader.cpp \
	utils/AmlMpBuffer.cpp \
	utils/AmlMpConfig.cpp \
	utils/AmlMpEventHandler.cpp \
	utils/AmlMpEventLooper.cpp \
	utils/AmlMpEventLooperRoster.cpp \
	utils/AmlMpLooper.cpp \
	utils/AmlMpMessage.cpp \
	utils/AmlMpRefBase.cpp \
	utils/AmlMpStrongPointer.cpp \
	utils/AmlMpThread.cpp \
	utils/AmlMpUtils.cpp \
	utils/Amlsysfsutils.cpp \
	utils/AmlMpChunkFifo.cpp \
	utils/AmlMpPlayerRoster.cpp \
	utils/json/lib_json/json_reader.cpp \
	utils/json/lib_json/json_value.cpp \
	utils/json/lib_json/json_writer.cpp \
	utils/AmlMpCodecCapability.cpp

AML_MP_SRCS := \
	$(AML_MP_PLAYER_SRC) \
	$(AML_MP_PLAYER_SYSTEM_SRC_$(PLATFORM_SDK_VERSION)) \
	$(AML_MP_CAS_SRC) \
	$(AML_MP_CAS_SYSTEM_SRC_$(PLATFORM_SDK_VERSION)) \
	$(AML_MP_DVR_SRC) \
	$(AML_MP_DEMUX_SRC) \
	$(AML_MP_UTILS_SRC) \

AML_MP_VENDOR_SRCS := \
	$(AML_MP_PLAYER_SRC) \
	$(AML_MP_PLAYER_VENDOR_SRC_$(PLATFORM_SDK_VERSION)) \
	$(AML_MP_CAS_SRC) \
	$(AML_MP_CAS_VENDOR_SRC_$(PLATFORM_SDK_VERSION)) \
	$(AML_MP_DVR_SRC) \
	$(AML_MP_DEMUX_SRC) \
	$(AML_MP_UTILS_SRC) \

#######################################
AML_MP_INC := $(LOCAL_PATH)/include \
	$(TOP)/vendor/amlogic/common/apps/LibTsPlayer/jni/include \
	$(AML_REFERENCE_PATH)/libdvr/include \
	$(AML_REFERENCE_PATH)/libdvr_release/include \
	$(AML_REFERENCE_PATH)/external/DTVKit/cas_hal/libamcas/include \
	$(TOP)/vendor/amlogic/common/mediahal_sdk/include \
	$(TOP)/hardware/amlogic/gralloc \
	$(TOP)/hardware/amlogic/media/amcodec/include \
	$(TOP)/vendor/amlogic/common/prebuilt/libmediadrm/ \
	$(AML_REFERENCE_PATH)/frameworks/services/subtitleserver/client \
	$(TOP)/frameworks/av/media/libmediametrics/include

AML_MP_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include \
	$(LOCAL_PATH) \
	$(TOP)/hardware/amlogic/media/amcodec/include \
	$(AML_REFERENCE_PATH)/libdvr/include \
	$(AML_REFERENCE_PATH)/libdvr_release/include \
	$(TOP)/vendor/amlogic/common/mediahal_sdk/include \

#######################################
AML_MP_CFLAGS := -DANDROID_PLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION) \
	-Werror -Wsign-compare \
	-DJSON_USE_EXCEPTION=0 \
	-DJSONCPP_NO_LOCALE_SUPPORT

AML_MP_SYSTEM_CFLAGS_28 := \
	-DHAVE_SUBTITLE \

ifeq ($(HAVE_CAS_HAL), true)
AML_MP_SYSTEM_CFLAGS_28 += \
	-DHAVE_CAS_HAL
endif

AML_MP_SYSTEM_CFLAGS_29 := \
	-DHAVE_SUBTITLE \
	#-DHAVE_VMXIPTV_CAS \
	#-DHAVE_CTC \

ifeq ($(HAVE_CAS_HAL), true)
AML_MP_SYSTEM_CFLAGS_29 += \
	-DHAVE_CAS_HAL
endif

AML_MP_VENDOR_CFLAGS_29 := \
	#-DHAVE_CTC \

AML_MP_SYSTEM_CFLAGS_ge_30 := \
	-DHAVE_SUBTITLE \

ifeq ($(HAVE_CAS_HAL), true)
AML_MP_SYSTEM_CFLAGS_ge_30 += \
	-DHAVE_CAS_HAL
endif

AML_MP_VENDOR_CFLAGS_28 := \
	-DHAVE_SUBTITLE \

ifeq ($(HAVE_CAS_HAL), true)
AML_MP_VENDOR_CFLAGS_28 += \
	-DHAVE_CAS_HAL
endif

ifeq ($(HAVE_TUNER_HAL), true)
AML_MP_SYSTEM_CFLAGS_ge_30 += \
	-DHAVE_TUNER_HAL
endif

AML_MP_VENDOR_CFLAGS_ge_30 := \
	-DHAVE_SUBTITLE \
	-DHAVE_AMUMEDIA

ifeq ($(HAVE_CAS_HAL), true)
AML_MP_VENDOR_CFLAGS_ge_30 += \
	-DHAVE_CAS_HAL
endif

ifeq ($(HAVE_WVIPTV_CAS), true)
AML_MP_VENDOR_CFLAGS_ge_30 += -DHAVE_WVIPTV_CAS_V2
AML_MP_SYSTEM_CFLAGS_ge_30 += -DHAVE_WVIPTV_CAS_V2
endif

ifeq ($(HAVE_VMXIPTV_CAS), true)
AML_MP_SYSTEM_CFLAGS_ge_30 += \
	-DHAVE_VMXIPTV_CAS_V2
endif

ifeq ($(HAVE_VMXWEB_CAS), true)
AML_MP_SYSTEM_CFLAGS_ge_30 += \
	-DHAVE_VMXWEB_CAS
endif

ifeq ($(HAVE_NAGRAWEB_CAS), true)
AML_MP_SYSTEM_CFLAGS_ge_30 += \
	-DHAVE_NAGRAWEB_CAS
endif

#######################################
AML_MP_SHARED_LIBS := \
	libutils \
	libcutils \
	liblog \
	libnativewindow \
	libui \
	libstagefright_foundation \
	libvideotunnel

AML_MP_SYSTEM_SHARED_LIBS_28 := \
	libmediahal_tsplayer.system \
	libSubtitleClient \
	libgui \
	libamgralloc_ext@2 \
	libamdvr.product \

AML_MP_SYSTEM_SHARED_LIBS_29 := \
	libmediahal_tsplayer.system \
	libSubtitleClient \
	libgui \
	libamgralloc_ext@2 \
	libamdvr.product \
	#libCTC_MediaProcessor \

AML_MP_SYSTEM_SHARED_LIBS_ge_30 := \
	libmediahal_tsplayer.system \
	libSubtitleClient \
	libgui \
	libamgralloc_ext \
	libamdvr.system \

ifeq ($(HAVE_TUNER_HAL), true)
AML_MP_SYSTEM_SHARED_LIBS_ge_30 += \
	android.hardware.tv.tuner@1.0 \
	android.hidl.memory@1.0 \
	libhidlbase \
	libhidlmemory \
	libstagefright \
	libmedia \
	libmediandk \
	libaudioclient \
	libfmq
endif

AML_MP_VENDOR_SHARED_LIBS_28 := \
	libSubtitleClient \
	libamdvr \
	libmediahal_tsplayer \
	libamgralloc_ext_vendor@2 \

AML_MP_VENDOR_SHARED_LIBS_29 := \
	libamdvr \
	libmediahal_tsplayer \
	libamgralloc_ext_vendor@2 \
	#libCTC_MediaProcessor.vendor \

AML_MP_VENDOR_SHARED_LIBS_ge_30 := \
	libSubtitleClient \
	libamdvr \
	libmediahal_tsplayer \
	libamgralloc_ext

#######################################
ifeq ($(HAVE_CAS_HAL), true)
AML_MP_SYSTEM_STATIC_LIBS_28 := \
	libam_cas_sys

AML_MP_VENDOR_STATIC_LIBS_28 := \
	libam_cas

AML_MP_SYSTEM_STATIC_LIBS_29 := \
	libam_cas

AML_MP_SYSTEM_STATIC_LIBS_ge_30 := \
	libam_cas_sys

AML_MP_VENDOR_STATIC_LIBS_ge_30 += \
	libam_cas
endif

###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := libaml_mp_sdk
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0 SPDX-license-identifier-MIT legacy_by_exception_only legacy_notice
LOCAL_LICENSE_CONDITIONS := by_exception_only notice restricted
LOCAL_NOTICE_FILE := $(LOCAL_PATH)/LICENSE
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(AML_MP_SRCS)
LOCAL_CFLAGS := $(AML_MP_CFLAGS) $(AML_MP_SYSTEM_CFLAGS_$(PLATFORM_SDK_VERSION))
LOCAL_C_INCLUDES := $(AML_MP_INC)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(AML_MP_EXPORT_C_INCLUDE_DIRS)
LOCAL_SHARED_LIBRARIES := $(AML_MP_SHARED_LIBS) $(AML_MP_SYSTEM_SHARED_LIBS_$(PLATFORM_SDK_VERSION))
LOCAL_STATIC_LIBRARIES := $(AML_MP_SYSTEM_STATIC_LIBS_$(PLATFORM_SDK_VERSION))
#LOCAL_WHOLE_STATIC_LIBRARIES :=
#LOCAL_LDFLAGS :=
ifeq (1, $(shell expr $(PLATFORM_SDK_VERSION) \>= 30))
LOCAL_SRC_FILES += $(AML_MP_PLAYER_SYSTEM_SRC_ge_30) $(AML_MP_CAS_SYSTEM_SRC_ge_30) $(AML_MP_DEMUX_SYSTEM_SRC_ge_30)
LOCAL_CFLAGS += $(AML_MP_SYSTEM_CFLAGS_ge_30)
LOCAL_SHARED_LIBRARIES += $(AML_MP_SYSTEM_SHARED_LIBS_ge_30)
LOCAL_STATIC_LIBRARIES += $(AML_MP_SYSTEM_STATIC_LIBS_ge_30)
LOCAL_SYSTEM_EXT_MODULE := true
endif
include $(BUILD_SHARED_LIBRARY)

###############################################################################
ifeq (1, $(shell expr $(PLATFORM_SDK_VERSION) \>= 28))
include $(CLEAR_VARS)
LOCAL_MODULE := libaml_mp_sdk.vendor
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0 SPDX-license-identifier-MIT legacy_by_exception_only legacy_notice
LOCAL_LICENSE_CONDITIONS := by_exception_only notice restricted
LOCAL_NOTICE_FILE := $(LOCAL_PATH)/LICENSE
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(AML_MP_VENDOR_SRCS)
LOCAL_CFLAGS := $(AML_MP_CFLAGS) $(AML_MP_VENDOR_CFLAGS_$(PLATFORM_SDK_VERSION))
LOCAL_C_INCLUDES := $(AML_MP_INC)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(AML_MP_EXPORT_C_INCLUDE_DIRS)
LOCAL_SHARED_LIBRARIES := $(AML_MP_SHARED_LIBS) $(AML_MP_VENDOR_SHARED_LIBS_$(PLATFORM_SDK_VERSION))
LOCAL_STATIC_LIBRARIES := $(AML_MP_VENDOR_STATIC_LIBS_$(PLATFORM_SDK_VERSION))
#LOCAL_WHOLE_STATIC_LIBRARIES :=
#LOCAL_LDFLAGS :=
ifeq (1, $(shell expr $(PLATFORM_SDK_VERSION) \>= 30))
LOCAL_CFLAGS += $(AML_MP_VENDOR_CFLAGS_ge_30)
LOCAL_SRC_FILES += $(AML_MP_PLAYER_VENDOR_SRC_ge_30) $(AML_MP_CAS_VENDOR_SRC_ge_30)
LOCAL_SHARED_LIBRARIES += $(AML_MP_VENDOR_SHARED_LIBS_ge_30)
LOCAL_STATIC_LIBRARIES += $(AML_MP_VENDOR_STATIC_LIBS_ge_30)
endif
include $(BUILD_SHARED_LIBRARY)
endif

###############################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))

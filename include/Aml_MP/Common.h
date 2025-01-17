/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef _AML_MP_COMMON_H_
#define _AML_MP_COMMON_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

///////////////////////////////////////////////////////////////////////////////
//typedef void* AML_MP_HANDLE;
#define AML_MP_INVALID_HANDLE   (0)
#define AML_MP_INVALID_PID      (0x1FFF)
#define AML_MP_MAX_PATH_SIZE    512
#define AML_MP_MAX_CODEC_CAPABILITY_STRING_SIZE  (1024)

#ifndef AML_MP_DEPRECATED
#define AML_MP_DEPRECATED __attribute((deprecated))
#endif

#ifndef __AML_MP_RESERVE_ALIGNED
#define __AML_MP_RESERVE_ALIGNED __attribute((aligned(sizeof(long))))
#endif

typedef void* AML_MP_PLAYER;
typedef void* AML_MP_DVRRECORDER;
typedef void* AML_MP_DVRPLAYER;
typedef void* AML_MP_CASSESSION;
typedef void* AML_MP_SECMEM;

///////////////////////////////////////////////////////////////////////////////
typedef enum {
    AML_MP_CHANNEL_ID_AUTO             = 0,
    AML_MP_CHANNEL_ID_MAIN,
    AML_MP_CHANNEL_ID_PIP,
} Aml_MP_ChannelId;

typedef enum {
    AML_MP_DEMUX_ID_DEFAULT         = -1,
    AML_MP_HW_DEMUX_ID_0            = 0,
    AML_MP_HW_DEMUX_ID_1,
    AML_MP_HW_DEMUX_ID_2,
    AML_MP_HW_DEMUX_ID_3,
    AML_MP_HW_DEMUX_ID_4,
    AML_MP_HW_DEMUX_ID_5,
    AML_MP_HW_DEMUX_ID_6,
    AML_MP_HW_DEMUX_ID_7,
    AML_MP_HW_DEMUX_ID_MAX,
    AML_MP_HW_DEMUX_NB              = (AML_MP_HW_DEMUX_ID_MAX - AML_MP_HW_DEMUX_ID_0),
} Aml_MP_DemuxId;

typedef enum {
    AML_MP_DEMUX_SOURCE_TS0,  /**< Hardware TS input port 0.*/
    AML_MP_DEMUX_SOURCE_TS1,  /**< Hardware TS input port 1.*/
    AML_MP_DEMUX_SOURCE_TS2,  /**< Hardware TS input port 2.*/
    AML_MP_DEMUX_SOURCE_TS3,  /**< Hardware TS input port 3.*/
    AML_MP_DEMUX_SOURCE_TS4,  /**< Hardware TS input port 4.*/
    AML_MP_DEMUX_SOURCE_TS5,  /**< Hardware TS input port 5.*/
    AML_MP_DEMUX_SOURCE_TS6,  /**< Hardware TS input port 6.*/
    AML_MP_DEMUX_SOURCE_TS7,  /**< Hardware TS input port 7.*/
    AML_MP_DEMUX_SOURCE_DMA0,  /**< DMA input port 0.*/
    AML_MP_DEMUX_SOURCE_DMA1,  /**< DMA input port 1.*/
    AML_MP_DEMUX_SOURCE_DMA2,  /**< DMA input port 2.*/
    AML_MP_DEMUX_SOURCE_DMA3,  /**< DMA input port 3.*/
    AML_MP_DEMUX_SOURCE_DMA4,  /**< DMA input port 4.*/
    AML_MP_DEMUX_SOURCE_DMA5,  /**< DMA input port 5.*/
    AML_MP_DEMUX_SOURCE_DMA6,  /**< DMA input port 6.*/
    AML_MP_DEMUX_SOURCE_DMA7,  /**< DMA input port 7.*/
    AML_MP_DEMUX_SECSOURCE_DMA0, /**< DMA secure port 0.*/
    AML_MP_DEMUX_SECSOURCE_DMA1, /**< DMA secure port 1.*/
    AML_MP_DEMUX_SECSOURCE_DMA2, /**< DMA secure port 2.*/
    AML_MP_DEMUX_SECSOURCE_DMA3, /**< DMA secure port 3.*/
    AML_MP_DEMUX_SECSOURCE_DMA4, /**< DMA secure port 4.*/
    AML_MP_DEMUX_SECSOURCE_DMA5, /**< DMA secure port 5.*/
    AML_MP_DEMUX_SECSOURCE_DMA6, /**< DMA secure port 6.*/
    AML_MP_DEMUX_SECSOURCE_DMA7, /**< DMA secure port 7.*/
    AML_MP_DEMUX_SOURCE_DMA0_1,  /**< DMA input port 0_1.*/
    AML_MP_DEMUX_SOURCE_DMA1_1,   /**< DMA input port 1_1.*/
    AML_MP_DEMUX_SOURCE_DMA2_1,  /**< DMA input port 2_1.*/
    AML_MP_DEMUX_SOURCE_DMA3_1,   /**< DMA input port 3_1.*/
    AML_MP_DEMUX_SOURCE_DMA4_1,  /**< DMA input port 4_1.*/
    AML_MP_DEMUX_SOURCE_DMA5_1,   /**< DMA input port 5_1.*/
    AML_MP_DEMUX_SOURCE_DMA6_1,  /**< DMA input port 6_1.*/
    AML_MP_DEMUX_SOURCE_DMA7_1,   /**< DMA input port 7_1.*/
    AML_MP_DEMUX_SECSOURCE_DMA0_1, /**< DMA secure port 0_1.*/
    AML_MP_DEMUX_SECSOURCE_DMA1_1, /**< DMA secure port 1_1.*/
    AML_MP_DEMUX_SECSOURCE_DMA2_1, /**< DMA secure port 2_1.*/
    AML_MP_DEMUX_SECSOURCE_DMA3_1, /**< DMA secure port 3_1.*/
    AML_MP_DEMUX_SECSOURCE_DMA4_1, /**< DMA secure port 4_1.*/
    AML_MP_DEMUX_SECSOURCE_DMA5_1, /**< DMA secure port 5_1.*/
    AML_MP_DEMUX_SECSOURCE_DMA6_1, /**< DMA secure port 6_1.*/
    AML_MP_DEMUX_SECSOURCE_DMA7_1,  /**< DMA secure port 7_1.*/
    AML_MP_DEMUX_SOURCE_TS0_1, /**< DMA secure port 0_1.*/
    AML_MP_DEMUX_SOURCE_TS1_1, /**< DMA secure port 1_1.*/
    AML_MP_DEMUX_SOURCE_TS2_1, /**< DMA secure port 2_1.*/
    AML_MP_DEMUX_SOURCE_TS3_1, /**< DMA secure port 3_1.*/
    AML_MP_DEMUX_SOURCE_TS4_1, /**< DMA secure port 4_1.*/
    AML_MP_DEMUX_SOURCE_TS5_1, /**< DMA secure port 5_1.*/
    AML_MP_DEMUX_SOURCE_TS6_1, /**< DMA secure port 6_1.*/
    AML_MP_DEMUX_SOURCE_TS7_1, /**< DMA secure port 7_1.*/
} Aml_MP_DemuxSource;

typedef enum {
    AML_MP_DEMUX_MEM_SEC_NONE       = 0,
    AML_MP_DEMUX_MEM_SEC_LEVEL1     = (1 << 10),
    AML_MP_DEMUX_MEM_SEC_LEVEL2     = (2 << 10),
    AML_MP_DEMUX_MEM_SEC_LEVEL3     = (3 << 10),
    AML_MP_DEMUX_MEM_SEC_LEVEL4     = (4 << 10),
    AML_MP_DEMUX_MEM_SEC_LEVEL5     = (5 << 10),
    AML_MP_DEMUX_MEM_SEC_LEVEL6     = (6 << 10),
    AML_MP_DEMUX_MEM_SEC_LEVEL7     = (7 << 10),
} Aml_MP_DemuxMemSecLevel;

///////////////////////////////////////////////////////////////////////////////
typedef enum {
    AML_MP_INPUT_SOURCE_TS_DEMOD,
    AML_MP_INPUT_SOURCE_TS_MEMORY,
    AML_MP_INPUT_SOURCE_ES_MEMORY,
    AML_MP_INPUT_SOURCE_USBCAM,
} Aml_MP_InputSourceType;

typedef enum {
    AML_MP_INPUT_STREAM_NORMAL,
    AML_MP_INPUT_STREAM_ENCRYPTED,
    AML_MP_INPUT_STREAM_SECURE_MEMORY,
} Aml_MP_InputStreamType;

/**
 * @brief Aml_MP_Option
 *
 * \param AML_MP_OPTION_PREFER_TUNER_HAL
 *      playing through TunerHal, playbackpipeline is:
 *      TunerHal + MediaCodec + AudioTrack
 * \param AML_MP_OPTION_MONITOR_PID_CHANGE
 *      monitor A/V pid change when playing, and report
 *      AML_MP_PLAYER_EVENT_PID_CHANGED when pid changed
 * \param AML_MP_OPTION_PREFER_CTC
 *      choose liveplayer internally
 * \param AML_MP_OPTION_REPORT_SUBTITLE_DATA
 *      report subtitle data
 * \param AML_MP_OPTION_DVR_PLAYBACK
 *      playing pvr case
 */
typedef enum {
    AML_MP_OPTION_PREFER_TUNER_HAL      = 1 << 0,
    AML_MP_OPTION_MONITOR_PID_CHANGE    = 1 << 1,
    AML_MP_OPTION_PREFER_CTC            = 1 << 2,
    AML_MP_OPTION_REPORT_SUBTITLE_DATA  = 1 << 3,
    AML_MP_OPTION_DVR_PLAYBACK          = 1 << 31,
} Aml_MP_Option;

typedef enum {
    AML_MP_INPUT_BUFFER_TYPE_NORMAL,
    AML_MP_INPUT_BUFFER_TYPE_SECURE,
    AML_MP_INPUT_BUFFER_TYPE_TVP,
} Aml_MP_InputBufferType;

typedef struct {
    Aml_MP_InputBufferType type;
    uint8_t* address;
    size_t size;
} Aml_MP_Buffer;

////////////////////////////////////////
typedef enum {
    AML_MP_CODEC_UNKNOWN = -1,
    AML_MP_VIDEO_CODEC_BASE = 0,
    AML_MP_VIDEO_CODEC_MPEG12,
    AML_MP_VIDEO_CODEC_MPEG4,
    AML_MP_VIDEO_CODEC_H264,
    AML_MP_VIDEO_CODEC_VC1,
    AML_MP_VIDEO_CODEC_AVS,
    AML_MP_VIDEO_CODEC_HEVC,
    AML_MP_VIDEO_CODEC_VP9,
    AML_MP_VIDEO_CODEC_AVS2,
    AML_MP_VIDEO_CODEC_MJPEG,
    AML_MP_VIDEO_CODEC_AV1,
    AML_MP_VIDEO_CODEC_DVES_AVC,           // DVES_AVC
    AML_MP_VIDEO_CODEC_DVES_HEVC,          // DVES_HEVC
    AML_MP_VIDEO_CODEC_MAX,

    AML_MP_AUDIO_CODEC_BASE = 1000,
    AML_MP_AUDIO_CODEC_MP2,                // MPEG audio
    AML_MP_AUDIO_CODEC_MP3,                // MP3
    AML_MP_AUDIO_CODEC_AC3,                // AC3
    AML_MP_AUDIO_CODEC_EAC3,               // DD PLUS
    AML_MP_AUDIO_CODEC_DTS,                // DD PLUS
    AML_MP_AUDIO_CODEC_AAC,                // AAC
    AML_MP_AUDIO_CODEC_LATM,               // AAC LATM
    AML_MP_AUDIO_CODEC_PCM,                // PCM
    AML_MP_AUDIO_CODEC_AC4,                // AC4
    AML_MP_AUDIO_CODEC_FLAC,               // FLAC
    AML_MP_AUDIO_CODEC_VORBIS,             // VORBIS
    AML_MP_AUDIO_CODEC_OPUS,               // OPUS
    AML_MP_AUDIO_CODEC_PCM_ADPCM_IMA_WAV,  // ADPCM
    AML_MP_AUDIO_CODEC_MAX,

    AML_MP_SUBTITLE_CODEC_BASE = 2000,
    AML_MP_SUBTITLE_CODEC_CC,
    AML_MP_SUBTITLE_CODEC_SCTE27,
    AML_MP_SUBTITLE_CODEC_DVB,
    AML_MP_SUBTITLE_CODEC_TELETEXT,
    AML_MP_SUBTITLE_CODEC_TTML,   //TTML Subtitle Support
    AML_MP_SUBTITLE_CODEC_ASS,
    AML_MP_SUBTITLE_CODEC_SUBRIP,
    AML_MP_SUBTITLE_CODEC_MAX,
} Aml_MP_CodecID;

typedef struct {
    uint16_t                pid;
    Aml_MP_CodecID          videoCodec;
    uint32_t                width;
    uint32_t                height;
    uint32_t                frameRate;
    uint8_t                 extraData[512];
    uint32_t                extraDataSize;
    Aml_MP_DemuxMemSecLevel secureLevel;
    long                   reserved[8];
} Aml_MP_VideoParams;

typedef struct {
    uint16_t                pid;
    Aml_MP_CodecID          audioCodec;
    uint32_t                nChannels;
    uint32_t                nSampleRate;
    uint8_t                 extraData[512];
    uint32_t                extraDataSize;
    Aml_MP_DemuxMemSecLevel secureLevel;
    long                    reserved[8];
} Aml_MP_AudioParams;

////////////////////////////////////////
typedef struct {
    int pid;
    Aml_MP_CodecID subtitleCodec;
    Aml_MP_CodecID videoFormat;        //cc
    int channelId;                     //cc
    int ancillaryPageId;               //dvb
    int compositionPageId;             //dvb
    long reserved[8];
} Aml_MP_SubtitleParams;

////////////////////////////////////////
/**\brief Service type of the program*/
typedef enum {
    AML_MP_CAS_SERVICE_LIVE_PLAY,       /**< Live playing.*/
    AML_MP_CAS_SERVICE_PVR_RECORDING,   /**< PVR recording.*/
    AML_MP_CAS_SERVICE_PVR_PLAY,        /**< PVR playback.*/
    AML_MP_CAS_SERVICE_PVR_TIMESHIFT_RECORDING,   /**< PVR recording for timeshift.*/
    AML_MP_CAS_SERVICE_PVR_TIMESHIFT_PLAY,        /**< PVR play for timeshift.*/

    AML_MP_CAS_SERVICE_TYPE_IPTV = 0x80,
    AML_MP_CAS_SERVICE_VERIMATRIX_IPTV, /**< Verimatrix IPTV*/
    AML_MP_CAS_SERVICE_VERIMATRIX_WEB,  /**<verimatrix WEB*/
    AML_MP_CAS_SERVICE_WIDEVINE,        /**<widevine*/
    AML_MP_CAS_SERVICE_NAGRA_WEB,       /**<Nagra web drm play*/
    AML_MP_CAS_SERVICE_TYPE_INVALID = 0xFF,     /**< Invalid type.*/
} Aml_MP_CASServiceType;

//add for get url info from setdatasource,such as wv
typedef struct {
    char license_url[AML_MP_MAX_PATH_SIZE];
    char provision_url[AML_MP_MAX_PATH_SIZE];
    char request_header[AML_MP_MAX_PATH_SIZE];
    char request_body[AML_MP_MAX_PATH_SIZE];
    char content_type[AML_MP_MAX_PATH_SIZE];
}Aml_MP_IptvCasHeaders;

#define MAX_ECM_PIDS_NUM 3
typedef struct {
    Aml_MP_CodecID videoCodec;
    Aml_MP_CodecID audioCodec;
    unsigned int  caSystemId;
    int videoPid;
    int audioPid;
    int ecmPid[MAX_ECM_PIDS_NUM];
#define AUDIO_ECM_PID_INDEX 0
#define VIDEO_ECM_PID_INDEX 1

    Aml_MP_DemuxId demuxId;

    //add for vmx get url info
    char serverAddress[AML_MP_MAX_PATH_SIZE];
    int serverPort;
    char keyPath[AML_MP_MAX_PATH_SIZE];

    //add for private data to cas
    //should care audio and video not same
    uint8_t private_data[AML_MP_MAX_PATH_SIZE];
    size_t private_size;

    Aml_MP_IptvCasHeaders headers;
    long reserved[8];
} Aml_MP_IptvCASParams;

////////////////////////////////////////
// Deprecated begin
typedef enum {
    AML_MP_CAS_UNKNOWN,
    AML_MP_CAS_VERIMATRIX_IPTV,
    AML_MP_CAS_WIDEVINE,
    AML_MP_CAS_VERIMATRIX_WEB,
} Aml_MP_CASType;

typedef struct {
    Aml_MP_CodecID videoCodec;
    Aml_MP_CodecID audioCodec;
    int videoPid;
    int audioPid;
    int ecmPid;
    Aml_MP_DemuxId demuxId;

    char serverAddress[100];
    int serverPort;
    char keyPath[100];
} Aml_MP_IptvCasParam;

typedef struct {
    Aml_MP_CASType type;
    union {
        Aml_MP_IptvCasParam iptvCasParam;
        struct {
            uint8_t data[1024];
            size_t size;
        } casParam;
    } u;
} Aml_MP_CASParams AML_MP_DEPRECATED;
// Deprecated end

////////////////////////////////////////
typedef enum {
    AML_MP_STREAM_TYPE_UNKNOWN = 0,
    AML_MP_STREAM_TYPE_VIDEO,
    AML_MP_STREAM_TYPE_AUDIO,
    AML_MP_STREAM_TYPE_AD,
    AML_MP_STREAM_TYPE_SUBTITLE,
    AML_MP_STREAM_TYPE_TELETEXT,
    AML_MP_STREAM_TYPE_ECM,
    AML_MP_STREAM_TYPE_EMM,
    AML_MP_STREAM_TYPE_SECTION,
    AML_MP_STREAM_TYPE_PCR,
    AML_MP_STREAM_TYPE_STC,
    AML_MP_STREAM_TYPE_NB,
} Aml_MP_StreamType;

/* information acquisition flags for audio and video */
typedef enum {
    AML_MP_STREAM_TYPE_MASK_VIDEO    = 1 << AML_MP_STREAM_TYPE_VIDEO,
    AML_MP_STREAM_TYPE_MASK_AUDIO    = 1 << AML_MP_STREAM_TYPE_AUDIO,
    AML_MP_STREAM_TYPE_MASK_AD       = 1 << AML_MP_STREAM_TYPE_AD,
    AML_MP_STREAM_TYPE_MASK_SUBTITLE = 1 << AML_MP_STREAM_TYPE_SUBTITLE,
}Aml_MP_StreamTypeMask;

typedef struct {
    uint8_t *data;      // Caller to provide buffer pointer
    size_t dataLength;    // The length of the buffer.
    size_t actualLength;  // Copy the length of the actual json
    uint32_t streamTypeMask;  // value from Aml_MP_StreamTypeMask
} Aml_MP_AvInfo;

typedef enum  {
    AML_MP_DECODING_STATE_STOPPED           = 0,
    AML_MP_DECODING_STATE_START_PENDING     = 1,
    AML_MP_DECODING_STATE_STARTED           = 2,
    AML_MP_DECODING_STATE_PAUSED            = 3,
} AML_MP_DecodingState;

////////////////////////////////////////
typedef struct {
    int size;
    int dataLen;
    int bufferedMs;
    long reserved[8];
} Aml_MP_BufferItem;

typedef struct {
    Aml_MP_BufferItem audioBuffer;
    Aml_MP_BufferItem videoBuffer;
    Aml_MP_BufferItem subtitleBuffer;
} Aml_MP_BufferStat;

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
typedef enum {
    //set/get
    AML_MP_PLAYER_PARAMETER_SET_BASE        = 0x1000,
    AML_MP_PLAYER_PARAMETER_VIDEO_DISPLAY_MODE,             //setVideoDisplayMode(Aml_MP_VideoDisplayMode*)
    AML_MP_PLAYER_PARAMETER_BLACK_OUT,                      //setVideoBlackOut(bool*)
    AML_MP_PLAYER_PARAMETER_VIDEO_DECODE_MODE,              //setVideoDecodeMode(Aml_MP_VideoDecodeMode*)
    AML_MP_PLAYER_PARAMETER_VIDEO_PTS_OFFSET,               //setVideoPtsOffset(int* ms)
    AML_MP_PLAYER_PARAMETER_AUDIO_OUTPUT_MODE,              //setAudioOutputMode(Aml_MP_AudioOutputMode*)
    AML_MP_PLAYER_PARAMETER_AUDIO_OUTPUT_DEVICE,            //setAudioOutputDevice(Aml_MP_AudioOutputDevice*)
    AML_MP_PLAYER_PARAMETER_AUDIO_PTS_OFFSET,               //setAudioPtsOffset(int* ms)
    AML_MP_PLAYER_PARAMETER_AUDIO_BALANCE,                  //setAudioBalance(Aml_MP_AudioBalance*)
    AML_MP_PLAYER_PARAMETER_AUDIO_MUTE,                     //setAudioMute(bool*)
    AML_MP_PLAYER_PARAMETER_CREATE_PARAMS,                  //setCreateParams(Aml_MP_PlayerCreateParams*)

    AML_MP_PLAYER_PARAMETER_NETWORK_JITTER,                 //setNetworkJitter(int* ms)

    AML_MP_PLAYER_PARAMETER_AD_STATE,                       //setADState(int*)
    AML_MP_PLAYER_PARAMETER_AD_MIX_LEVEL,                   //setADMixLevel(Aml_MP_ADVolume*)

    AML_MP_PLAYER_PARAMETER_WORK_MODE,                      //setWorkMode(Aml_MP_PlayerWorkMode*)
    AML_MP_PLAYER_PARAMETER_VIDEO_WINDOW_ZORDER,            //setZorder(int*)
    AML_MP_PLAYER_PARAMETER_TELETEXT_CONTROL,               //amlsub_TeletextControl(AML_MP_TeletextCtrlParam*)

    AML_MP_PLAYER_PARAMETER_VENDOR_ID,                      //setVendorID(int*)
    AML_MP_PLAYER_PARAMETER_VIDEO_TUNNEL_ID,                //setVideoTunnelID(int*)
    AML_MP_PLAYER_PARAMETER_SURFACE_HANDLE,                 //setSurface(void*)
    AML_MP_PLAYER_PARAMETER_AUDIO_PRESENTATION_ID,          //setPresentationId(int*)
    AML_MP_PLAYER_PARAMETER_USE_TIF,                        //setUseTif(bool*)
    AML_MP_PLAYER_PARAMETER_SPDIF_PROTECTION,               //SetSPDIFProtection(int*)
    AML_MP_PLAYER_PARAMETER_VIDEO_CROP,                     //setVideoCrop(Aml_MP_Rect*)
    AML_MP_PLAYER_PARAMETER_VIDEO_ERROR_RECOVERY_MODE,      //setVideoErrorRecoveryMode(Aml_MP_VideoErrorRecoveryMode*)
    AML_MP_PLAYER_PARAMETER_AUDIO_LANGUAGE,                 //setAudioLanguage(Aml_MP_AudioLanguage*)
    AML_MP_PLAYER_PARAMETER_VIDEO_AFD_ASPECT_MODE,          //setVideoAFDAspectMode(Aml_MP_VideoAFDAspectMode *)
    AML_MP_PLAYER_PARAMETER_LIBDVR_FAKE_PID,                //setLibDvrFakePID(int*)
    AML_MP_PLAYER_PARAMETER_AUDIO_BLOCK_ALIGN,              //setAudioBlockAlign(int*)

    //get only
    AML_MP_PLAYER_PARAMETER_GET_BASE        = 0x2000,
    AML_MP_PLAYER_PARAMETER_VIDEO_INFO,                     //getVideoInfo(Aml_MP_VideoInfo*)
    AML_MP_PLAYER_PARAMETER_VIDEO_DECODE_STAT,              //getVdecStat(Aml_MP_VdecStat*)
    AML_MP_PLAYER_PARAMETER_AUDIO_INFO,                     //getAudioInfo(Aml_MP_AudioInfo*)
    AML_MP_PLAYER_PARAMETER_AUDIO_DECODE_STAT,              //getAudioDecStat(Aml_MP_AdecStat*)
    AML_MP_PLAYER_PARAMETER_SUBTITLE_INFO,                  //getSubtitleInfo(Aml_MP_SubtitleInfo*)
    AML_MP_PLAYER_PARAMETER_SUBTITLE_DECODE_STAT,           //getSubtitleDecStat(Aml_MP_SubDecStat*)
    AML_MP_PLAYER_PARAMETER_AD_INFO,                        //getADInfo(Aml_MP_AudioInfo*)
    AML_MP_PLAYER_PARAMETER_AD_DECODE_STAT,                 //getADDecodeStat(Aml_MP_AdecStat*)
    AML_MP_PLAYER_PARAMETER_INSTANCE_ID,                    //getInstanceId(uint32_t*)
    AML_MP_PLAYER_PARAMETER_SYNC_ID,                        //getSyncId(int32_t*)
    AML_MP_PLAYER_PARAMETER_VIDEO_SHOW_STATE,               //getVideoShowState(bool*)
    AML_MP_PLAYER_PARAMETER_AV_INFO_JSON,                   //getAVInfo(Aml_MP_AvInfo*)
    AML_MP_PLAYER_PARAMETER_TSPLAYER_HANDLE,                //getTsPlayerHandle(am_tsplayer_handle*)
} Aml_MP_PlayerParameterKey;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_VIDEO_DISPLAY_MODE
typedef enum {
    AML_MP_VIDEO_DISPLAY_MODE_NORMAL,
    AML_MP_VIDEO_DISPLAY_MODE_FULLSCREEN,
    AML_MP_VIDEO_DISPLAY_MODE_LETTER_BOX,
    AML_MP_VIDEO_DISPLAY_MODE_PAN_SCAN,
    AML_MP_VIDEO_DISPLAY_MODE_COMBINED,
    AML_MP_VIDEO_DISPLAY_MODE_WIDTHFULL,
    AML_MP_VIDEO_DISPLAY_MODE_HEIGHTFULL,
    AML_MP_VIDEO_DISPLAY_MODE_4_3_LETTER_BOX,
    AML_MP_VIDEO_DISPLAY_MODE_4_3_PAN_SCAN,
    AML_MP_VIDEO_DISPLAY_MODE_4_3_COMBINED,
    AML_MP_VIDEO_DISPLAY_MODE_16_9_IGNORE,
    AML_MP_VIDEO_DISPLAY_MODE_16_9_LETTER_BOX,
    AML_MP_VIDEO_DISPLAY_MODE_16_9_PAN_SCAN,
    AML_MP_VIDEO_DISPLAY_MODE_16_9_COMBINED,
    AML_MP_VIDEO_DISPLAY_MODE_CUSTOM,
} Aml_MP_VideoDisplayMode;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_VIDEO_DECODE_MODE
typedef enum {
    AML_MP_VIDEO_DECODE_MODE_NONE,
    AML_MP_VIDEO_DECODE_MODE_IONLY,
    AML_MP_VIDEO_DECODE_MODE_PAUSE,
    AML_MP_VIDEO_DECODE_MODE_PAUSE_NEXT,
} Aml_MP_VideoDecodeMode;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_PLAYER_WORK_MODE
typedef enum {
    AML_MP_PLAYER_MODE_NORMAL = 0,             // Normal mode
    AML_MP_PLAYER_MODE_CACHING_ONLY = 1,       // Only caching data, do not decode. Used in FCC
    AML_MP_PLAYER_MODE_DECODE_ONLY = 2         // Decode data but do not output
} Aml_MP_PlayerWorkMode;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_AUDIO_OUTPUT_MODE
typedef enum {
    AML_MP_AUDIO_OUTPUT_PCM,
    AML_MP_AUDIO_OUTPUT_PASSTHROUGH,
    AML_MP_AUDIO_OUTPUT_AUTO,
} Aml_MP_AudioOutputMode;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_AUDIO_OUTPUT_DEVICE
typedef enum {
    AML_MP_AUDIO_OUTPUT_DEVICE_NORMAL,
    AML_MP_AUDIO_OUTPUT_DEVICE_BT,
} Aml_MP_AudioOutputDevice;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_AUDIO_BALANCE
typedef enum {
    AML_MP_AUDIO_BALANCE_STEREO,
    AML_MP_AUDIO_BALANCE_LEFT,
    AML_MP_AUDIO_BALANCE_RIGHT,
    AML_MP_AUDIO_BALANCE_SWAP,
    AML_MP_AUDIO_BALANCE_LRMIX,
} Aml_MP_AudioBalance;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_AD_MIX_LEVEL
typedef struct {
    int masterVolume;
    int slaveVolume;
} Aml_MP_ADVolume;

////////////////////////////////////////
typedef enum {
    AML_MP_RATIO_4_3,
    AML_MP_RATIO_16_9,
    AML_MP_RATIO_UNDEFINED = 255
} Aml_MP_VideoRatio;

////////////////////////////////////////
typedef enum {
    AML_MP_VIDEO_AFD_ASPECT_MODE_NONE = -1,
    AML_MP_VIDEO_AFD_ASPECT_MODE_AUTO,
    AML_MP_VIDEO_AFD_ASPECT_MODE_4_3,  /* 16:9 video, on 4:3 display/scene -> Centre Cut-Out */
    AML_MP_VIDEO_AFD_ASPECT_MODE_16_9, /* 16:9 video, on 4:3 display/scene -> Letter Box */
    AML_MP_VIDEO_AFD_ASPECT_MODE_14_9,
    AML_MP_VIDEO_AFD_ASPECT_MODE_ZOOM,
    AML_MP_VIDEO_AFD_ASPECT_MODE_CUSTOM,
    AML_MP_VIDEO_AFD_ASPECT_MODE_MAX,
} Aml_MP_VideoAFDAspectMode;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_VIDEO_INFO
typedef struct {
    int width;
    int height;
    int frameRate;
    int bitrate;
    Aml_MP_VideoRatio ratio64;
    long reserved[8];
} Aml_MP_VideoInfo;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_VIDEO_DECODE_STAT
typedef struct {
    uint32_t num;
    uint32_t type;
    uint32_t size;
    uint32_t pts;
    uint32_t max_qp;
    uint32_t avg_qp;
    uint32_t min_qp;
    uint32_t max_skip;
    uint32_t avg_skip;
    uint32_t min_skip;
    uint32_t max_mv;
    uint32_t min_mv;
    uint32_t avg_mv;
    uint32_t decode_buffer;
} Aml_MP_VideoQos;

typedef struct {
    Aml_MP_VideoQos qos;
    uint32_t  decode_time_cost;/*us*/
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t frame_rate;
    uint32_t bit_depth_luma;//original bit_rate;
    uint32_t frame_dur;
    uint32_t bit_depth_chroma;//original frame_data;
    uint32_t error_count;
    uint32_t status;
    uint32_t frame_count;
    uint32_t error_frame_count;
    uint32_t drop_frame_count;
    uint64_t total_data;
    uint32_t double_write_mode;//original samp_cnt;
    uint32_t offset;
    uint32_t ratio_control;
    uint32_t vf_type;
    uint32_t signal_type;
    uint32_t pts;
    uint64_t pts_us64;
#if 0 // For av param report
    uint32_t i_decoded_frames; //i frames decoded
    uint32_t i_lost_frames;//i frames can not be decoded
    uint32_t i_concealed_frames;//i frames decoded but have some error
    uint32_t p_decoded_frames;
    uint32_t p_lost_frames;
    uint32_t p_concealed_frames;
    uint32_t b_decoded_frames;
    uint32_t b_lost_frames;
    uint32_t b_concealed_frames;
    uint32_t av_resynch_counter;
#endif
    long reserved[16];
} Aml_MP_VdecStat;


////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_AUDIO_INFO
typedef struct {
    uint32_t sample_rate;                  // Audio sample rate
    uint32_t channels;                     // Audio channels
    uint32_t channel_mask;                 // Audio channel mask
    uint32_t bitrate;                      // Audio bit rate
    long reserved[8];
} Aml_MP_AudioInfo;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_AUDIO_DECODE_STAT
typedef struct {
    uint32_t frame_count;
    uint32_t error_frame_count;
    uint32_t drop_frame_count;
    long reserved[8];
} Aml_MP_AdecStat;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_SUBTITLE_INFO
//AML_MP_PLAYER_EVENT_SUBTITLE_INFO
typedef struct {
    int what;
    int extra;
    long reserved[8];
}Aml_MP_SubtitleInfo;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_SUBTITLE_DECODE_STAT
typedef struct {
    uint32_t frameCount;
    uint32_t errorFrameCount;
    uint32_t dropFrameCount;
    long reserved[8];
} Aml_MP_SubDecStat;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_TELETEXT_CONTROL
typedef enum {
    AML_MP_TT_EVENT_INVALID          = -1,

    // These are the four FastText shortcuts, usually represented by red, green,
    // yellow and blue keys on the handset.
    AML_MP_TT_EVENT_QUICK_NAVIGATE_RED = 0,
    AML_MP_TT_EVENT_QUICK_NAVIGATE_GREEN,
    AML_MP_TT_EVENT_QUICK_NAVIGATE_YELLOW,
    AML_MP_TT_EVENT_QUICK_NAVIGATE_BLUE,

    // The ten numeric keys used to input page indexes.
    AML_MP_TT_EVENT_0,
    AML_MP_TT_EVENT_1,
    AML_MP_TT_EVENT_2,
    AML_MP_TT_EVENT_3,
    AML_MP_TT_EVENT_4,
    AML_MP_TT_EVENT_5,
    AML_MP_TT_EVENT_6,
    AML_MP_TT_EVENT_7,
    AML_MP_TT_EVENT_8,
    AML_MP_TT_EVENT_9,

    // This is the home key, which returns to the nominated index page for this service.
    AML_MP_TT_EVENT_INDEXPAGE,

    // These are used to quickly increment/decrement the page index.
    AML_MP_TT_EVENT_NEXTPAGE,
    AML_MP_TT_EVENT_PREVIOUSPAGE,

    // These are used to navigate the sub-pages when in 'hold' mode.
    AML_MP_TT_EVENT_NEXTSUBPAGE,
    AML_MP_TT_EVENT_PREVIOUSSUBPAGE,

    // These are used to traverse the page history (if caching requested).
    AML_MP_TT_EVENT_BACKPAGE,
    AML_MP_TT_EVENT_FORWARDPAGE,

    // This is used to toggle hold on the current page.
    AML_MP_TT_EVENT_HOLD,
    // Reveal hidden page content (as defined in EBU specification)
    AML_MP_TT_EVENT_REVEAL,
    // This key toggles 'clear' mode (page hidden until updated)
    AML_MP_TT_EVENT_CLEAR,
    // This key toggles 'clock only' mode (page hidden until updated)
    AML_MP_TT_EVENT_CLOCK,
    // Used to toggle transparent background ('video mix' mode)
    AML_MP_TT_EVENT_MIX_VIDEO,
    // Used to toggle double height top / double-height bottom / normal height display.
    AML_MP_TT_EVENT_DOUBLE_HEIGHT,
    // Functional enhancement may offer finer scrolling of double-height display.
    AML_MP_TT_EVENT_DOUBLE_SCROLL_UP,
    AML_MP_TT_EVENT_DOUBLE_SCROLL_DOWN,
    // Used to initiate/cancel 'timer' mode (clear and re-display page at set time)
    AML_MP_TT_EVENT_TIMER,
    AML_MP_TT_EVENT_GO_TO_PAGE,
    AML_MP_TT_EVENT_GO_TO_SUBTITLE
} Aml_MP_TeletextEvent;

typedef struct {
    int magazine;
    int page;
    Aml_MP_TeletextEvent event;
    char iso639_lang[4] __AML_MP_RESERVE_ALIGNED;
    long reserved[7];
} AML_MP_TeletextCtrlParam;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_VIDEO_CROP
typedef struct {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
} Aml_MP_Rect;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_VIDEO_ERROR_RECOVERY_MODE
typedef enum {
    AML_MP_VIDEO_ERROR_RECOVERY_DEFAULT,    //Use decoder default setting to process error frame
    AML_MP_VIDEO_ERROR_RECOVERY_DROP,       //Drop frame if any error detected
    AML_MP_VIDEO_ERROR_RECOVERY_NONE,       //Do nothing after decoding error frame
} Aml_MP_VideoErrorRecoveryMode;

////////////////////////////////////////
//AML_MP_PLAYER_PARAMETER_AUDIO_LANGUAGE
typedef struct {
    uint32_t            firstLanguage;
    uint32_t            secondLanguage;
    long                reserved[8];
} Aml_MP_AudioLanguage;

///////////////////////////////////////////////////////////////////////////////
typedef enum {
    AML_MP_AVSYNC_SOURCE_DEFAULT,
    AML_MP_AVSYNC_SOURCE_VIDEO,
    AML_MP_AVSYNC_SOURCE_AUDIO,
    AML_MP_AVSYNC_SOURCE_PCR,
    AML_MP_AVSYNC_SOURCE_NOSYNC,
} Aml_MP_AVSyncSource;

///////////////////////////////////////////////////////////////////////////////
enum {
    AML_MP_OK                           = 0,
    AML_MP_ERROR                        = (-2147483647-1),
    AML_MP_ERROR_NO_MEMORY              = -ENOMEM,                  // Not enough space
    AML_MP_ERROR_INVALID_OPERATION      = -ENOSYS,                  // Function not implemented
    AML_MP_ERROR_BAD_VALUE              = -EINVAL,                  // Invalid argument
    AML_MP_ERROR_BAD_TYPE               = (AML_MP_ERROR+1),
    AML_MP_ERROR_NAME_NOT_FOUND         = -ENOENT,                  // No such file or directory
    AML_MP_ERROR_PERMISSION_DENIED      = -EPERM,                   // Operation not permitted
    AML_MP_ERROR_NO_INIT                = -ENODEV,                  // No such device
    AML_MP_ERROR_ALREADY_EXISTS         = -EEXIST,                  // File exists
    AML_MP_ERROR_DEAD_OBJECT            = -EPIPE,                   // Broken pipe
    AML_MP_ERROR_FAILED_TRANSACTION     = (AML_MP_ERROR+2),
    AML_MP_ERROR_BAD_INDEX              = -EOVERFLOW,               // Value too large to be stored in data type
    AML_MP_ERROR_NOT_ENOUGH_DATA        = -ENODATA,                 // No message is available on the STREAM head read queue
    AML_MP_ERROR_WOULD_BLOCK            = -EWOULDBLOCK,             // Operation would block
    AML_MP_ERROR_TIMED_OUT              = -ETIMEDOUT,               // Connection timed out
    AML_MP_ERROR_UNKNOWN_TRANSACTION    = -EBADMSG,                 // Bad message
    AML_MP_ERROR_FDS_NOT_ALLOWED        = (AML_MP_ERROR+3),
    AML_MP_ERROR_UNEXPECTED_NULL        = (AML_MP_ERROR+4),
};

///////////////////////////////////////////////////////////////////////////////
#define AML_MP_DVR_STREAMS_COUNT 32

typedef struct {
    Aml_MP_StreamType   type;
    int                 pid;
    Aml_MP_CodecID      codecId;
    Aml_MP_DemuxMemSecLevel secureLevel;
    long                reserved[8];
} Aml_MP_DVRStream;

typedef struct {
    int nbStreams;
    Aml_MP_DVRStream streams[AML_MP_DVR_STREAMS_COUNT];
#define AML_MP_DVR_VIDEO_INDEX      0
#define AML_MP_DVR_AUDIO_INDEX      1
#define AML_MP_DVR_AD_INDEX         2
#define AML_MP_DVR_SUBTITLE_INDEX   3
#define AML_MP_DVR_PCR_INDEX        4
#define AML_MP_DVR_STREAM_NB        5
} Aml_MP_DVRStreamArray;

typedef struct {
  time_t              time;       /**< time duration, unit on ms*/
  loff_t              size;       /**< size*/
  uint32_t            pkts;       /**< number of ts packets*/
  long                reserved[8];
} Aml_MP_DVRSourceInfo;

///////////////////////////////////////////////////////////////////////////////
typedef enum {
    AML_MP_EVENT_UNKNOWN                            = 0x0000,
    AML_MP_PLAYER_EVENT_FIRST_FRAME                 = 0x0001,
    AML_MP_PLAYER_EVENT_AV_SYNC_DONE                = 0x0002,
    AML_MP_PLAYER_EVENT_DATA_LOSS                   = 0x0003,   //Demod data loss
    AML_MP_PLAYER_EVENT_DATA_RESUME                 = 0x0004,   //Demod data loss
    AML_MP_PLAYER_EVENT_SCRAMBLING                  = 0x0005,
    AML_MP_PLAYER_EVENT_USERDATA_AFD                = 0x0006,
    AML_MP_PLAYER_EVENT_USERDATA_CC                 = 0x0007,
    AML_MP_PLAYER_EVENT_PID_CHANGED                 = 0x0008,   //param: Aml_MP_PlayerEventPidChangeInfo
    AML_MP_PLAYER_EVENT_DECODER_DATA_LOSS           = 0x0009,   //Decoder data loss
    AML_MP_PLAYER_EVENT_DECODER_DATA_RESUME         = 0x000A,   //Decoder data resume

    // DVR player
    AML_MP_DVRPLAYER_EVENT_ERROR                = 0x1000,   /**< Signal a critical playback error*/
    AML_MP_DVRPLAYER_EVENT_TRANSITION_OK,                   /**< transition ok*/
    AML_MP_DVRPLAYER_EVENT_TRANSITION_FAILED,               /**< transition failed*/
    AML_MP_DVRPLAYER_EVENT_KEY_FAILURE,                     /**< key failure*/
    AML_MP_DVRPLAYER_EVENT_NO_KEY,                          /**< no key*/
    AML_MP_DVRPLAYER_EVENT_REACHED_BEGIN,                   /**< reached begin*/
    AML_MP_DVRPLAYER_EVENT_REACHED_END,                     /**< reached end*/
    AML_MP_DVRPLAYER_EVENT_NOTIFY_PLAYTIME,                 /**< notify play cur segment time ms*/
    AML_MP_DVRPLAYER_EVENT_TIMESHIFT_FR_REACHED_BEGIN = 0x100b,
    AML_MP_DVRPLAYER_EVENT_TIMESHIFT_FF_REACHED_END,

    // Video event
    AML_MP_PLAYER_EVENT_VIDEO_BASE              = 0x2000,
    AML_MP_PLAYER_EVENT_VIDEO_CHANGED,
    AML_MP_PLAYER_EVENT_VIDEO_DECODE_FIRST_FRAME,
    AML_MP_PLAYER_EVENT_VIDEO_OVERFLOW,
    AML_MP_PLAYER_EVENT_VIDEO_UNDERFLOW,
    AML_MP_PLAYER_EVENT_VIDEO_INVALID_TIMESTAMP,
    AML_MP_PLAYER_EVENT_VIDEO_INVALID_DATA,
    AML_MP_PLAYER_EVENT_VIDEO_ERROR_FRAME_COUNT,
    AML_MP_PLAYER_EVENT_VIDEO_UNSUPPORT,
    AML_MP_PLAYER_EVENT_VIDEO_INPUT_BUFFER_DONE,

    // Audio event
    AML_MP_PLAYER_EVENT_AUDIO_BASE              = 0x3000,
    AML_MP_PLAYER_EVENT_AUDIO_CHANGED,
    AML_MP_PLAYER_EVENT_AUDIO_DECODE_FIRST_FRAME,
    AML_MP_PLAYER_EVENT_AUDIO_OVERFLOW,
    AML_MP_PLAYER_EVENT_AUDIO_UNDERFLOW,
    AML_MP_PLAYER_EVENT_AUDIO_INVALID_TIMESTAMP,
    AML_MP_PLAYER_EVENT_AUDIO_INVALID_DATA,
    AML_MP_PLAYER_EVENT_AUDIO_INPUT_BUFFER_DONE,

    // Subtitle event
    AML_MP_PLAYER_EVENT_SUBTITLE_BASE           = 0x4000,
    AML_MP_PLAYER_EVENT_SUBTITLE_DATA,                      //param: Aml_MP_SubtitleData*
    AML_MP_PLAYER_EVENT_SUBTITLE_AVAIL,                     //param: int*
    AML_MP_PLAYER_EVENT_SUBTITLE_DIMENSION,                 //param: Aml_MP_SubtitleDimension*
    AML_MP_PLAYER_EVENT_SUBTITLE_AFD_EVENT,                 //param: int*
    AML_MP_PLAYER_EVENT_SUBTITLE_CHANNEL_UPDATE,            //param: Aml_MP_SubtitleChannelUpdate*
    AML_MP_PLAYER_EVENT_SUBTITLE_LANGUAGE,                  //param: char[4]
    AML_MP_PLAYER_EVENT_SUBTITLE_INFO,                      //param: Aml_MP_SubtitleInfo*
    AML_MP_PLAYER_EVENT_SUBTITLE_LOSEDATA,
    AML_MP_PLAYER_EVENT_SUBTITLE_TIMEOUT,
    AML_MP_PLAYER_EVENT_SUBTITLE_INVALID_TIMESTAMP,
    AML_MP_PLAYER_EVENT_SUBTITLE_INVALID_DATA,

    //medialayer
    AML_MP_MEDIAPLAYER_EVENT_BASE                 = 0x5000,
    AML_MP_MEDIAPLAYER_EVENT_PLAYBACK_COMPLETE,
    AML_MP_MEDIAPLAYER_EVENT_PLAYBACK_SOF,
    AML_MP_MEDIAPLAYER_EVENT_PREPARED,
    AML_MP_MEDIAPLAYER_EVENT_STOPPED,
    AML_MP_MEDIAPLAYER_EVENT_MEDIA_ERROR,
    AML_MP_MEDIAPLAYER_EVENT_FIRST_FRAME_TOGGLED,
    AML_MP_MEDIAPLAYER_EVENT_BUFFERING_START,
    AML_MP_MEDIAPLAYER_EVENT_BUFFERING_END,
    AML_MP_MEDIAPLAYER_EVENT_VIDEO_UNSUPPORT,
    AML_MP_MEDIAPLAYER_EVENT_AUDIO_UNSUPPORT,
} Aml_MP_PlayerEventType;

//AML_MP_PLAYER_EVENT_VIDEO_CHANGED,
typedef struct {
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t frame_rate;
    uint32_t frame_aspectratio;
    long     reserved[8];
} Aml_MP_PlayerEventVideoFormat;

//AML_MP_PLAYER_EVENT_AUDIO_CHANGED,
typedef struct {
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t channel_mask;
    long     reserved[8];
} Aml_MP_PlayerEventAudioFormat;

//AML_MP_PLAYER_EVENT_SCRAMBLING,
typedef struct {
    Aml_MP_StreamType type;
    char scramling;
    long reserved[8];
} Aml_MP_PlayerEventScrambling;

//AML_MP_PLAYER_EVENT_PID_CHANGED
typedef struct {
    int programPid;
    int programNumber;
    Aml_MP_StreamType type;
    int oldStreamPid;
    int newStreamPid;
    union {
        Aml_MP_VideoParams      videoParams;
        Aml_MP_AudioParams      audioParams;
        Aml_MP_SubtitleParams   subtitleParams;
    } u;
    long reserved[8];
} Aml_MP_PlayerEventPidChangeInfo;


//AML_MP_PLAYER_EVENT_USERDATA_AFD,
//AML_MP_PLAYER_EVENT_USERDATA_CC,
typedef struct {
    uint8_t  *data;
    size_t   len;
    long     reserved[8];
} Aml_MP_PlayerEventMpegUserData;

////////////////////////////////////////
//AML_MP_PLAYER_EVENT_SUBTITLE_DATA
typedef enum {
    AML_MP_SUB_DATA_TYPE_STRING = 0,
    AML_MP_SUB_DATA_TYPE_CC_JSON = 1,
    AML_MP_SUB_DATA_TYPE_BITMAP = 2,
    AML_MP_SUB_DATA_TYPE_POSITON_BITMAP = 4,
    AML_MP_SUB_DATA_TYPE_POSITION_BITMAP = 4,
    AML_MP_SUB_DATA_TYPE_UNKNOWN = 0xFF,
}AML_MP_SubtitleDataType;

typedef struct {
    const char *data;
    int size;
    AML_MP_SubtitleDataType type;
    int x;
    int y;
    int width;
    int height;
    int videoWidth;
    int videoHeight;
    int showing;
    long reserved[8];
}Aml_MP_SubtitleData;

////////////////////////////////////////
//AML_MP_PLAYER_EVENT_SUBTITLE_DIMENSION
typedef struct {
    uint32_t width;
    uint32_t height;
    long     reserved[8];
} Aml_MP_SubtitleDimension;

////////////////////////////////////////
//AML_MP_PLAYER_EVENT_SUBTITLE_CHANNEL_UPDATE
typedef struct {
    int event;
    int id;
    long reserved[8];
}Aml_MP_SubtitleChannelUpdate;

typedef void (*Aml_MP_PlayerEventCallback)(void* userData, Aml_MP_PlayerEventType event, int64_t param);

typedef struct ANativeWindow ANativeWindow;

#endif

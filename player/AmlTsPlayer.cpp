/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#define LOG_TAG "AmlTsPlayer"
#include <utils/AmlMpLog.h>
#include "AmlTsPlayer.h"
#include <AmTsPlayer.h>
#include <utils/AmlMpUtils.h>
#ifdef ANDROID
#include <system/window.h>
#include <amlogic/am_gralloc_ext.h>
#ifndef __ANDROID_VNDK__
#include <gui/Surface.h>
#endif
#endif
#include "Aml_MP_PlayerImpl.h"

namespace aml_mp {

void AmlTsPlayer::AudioEsDataFeedThread::start()
{
    run("AudioEsDataFeedThread");
}
int AmlTsPlayer::AudioEsDataFeedThread::writeEsData(const uint8_t* buffer, size_t size, int64_t pts)
{
    std::unique_lock<std::mutex> _l(mLock);
    if (mAudioEsQueue.size() > 100) {
        return -1;
    }
    struct AudioEsBuffer audioBuffer = {
        .addr = (uint8_t*)buffer,
        .size = size,
        .pts = pts,
    };
    mAudioEsQueue.push(audioBuffer);
    return size;
}
void AmlTsPlayer::AudioEsDataFeedThread::pause()
{
    std::unique_lock<std::mutex> _l(mLock);
    mPaused = true;
}
void AmlTsPlayer::AudioEsDataFeedThread::flush()
{
    std::unique_lock<std::mutex> _l(mLock);
    struct AudioEsBuffer audioBuffer;
    while (mAudioEsQueue.size() > 0) {
        audioBuffer = mAudioEsQueue.front();
        mAudioEsQueue.pop();
        mPlayer->notifyListener(AML_MP_PLAYER_EVENT_AUDIO_INPUT_BUFFER_DONE, (int64_t)audioBuffer.addr);
    }
}

void AmlTsPlayer::AudioEsDataFeedThread::resume()
{
    std::unique_lock<std::mutex> _l(mLock);
    mPaused = false;
}
void AmlTsPlayer::AudioEsDataFeedThread::stop()
{
    flush();
    requestExitAndWait();
}

bool AmlTsPlayer::AudioEsDataFeedThread::threadLoop() {
    while (!exitPending()) {
        struct AudioEsBuffer audioBuffer;
        bool hasData = false;
        {
            std::unique_lock<std::mutex> _l(mLock);
            if (!mPaused && mAudioEsQueue.size() > 0) {
                audioBuffer = mAudioEsQueue.front();
                hasData = true;
            }
        }
        if (hasData) {
            int ret = mPlayer->writeEsData_l(AML_MP_STREAM_TYPE_AUDIO, audioBuffer.addr, audioBuffer.size, audioBuffer.pts);
            if (ret >= 0) {
                {
                    std::unique_lock<std::mutex> _l(mLock);
                    mAudioEsQueue.pop();
                }
                mPlayer->notifyListener(AML_MP_PLAYER_EVENT_AUDIO_INPUT_BUFFER_DONE, (int64_t)audioBuffer.addr);
            }
        }
        usleep(1000); // todo, should use condition variable
    }
    return false;
}

AmlTsPlayer::AmlTsPlayer(Aml_MP_PlayerCreateParams* createParams, int instanceId)
: aml_mp::AmlPlayerBase(createParams, instanceId)
{
    snprintf(mName, sizeof(mName), "%s_%d", LOG_TAG, instanceId);

    AmlMpPlayerRoster::instance().signalAmTsPlayerId(instanceId);

    MLOGI("demuxId: %s", mpDemuxId2Str(createParams->demuxId));
    if (createParams->demuxId == AML_MP_DEMUX_ID_DEFAULT) {
        createParams->demuxId = AML_MP_HW_DEMUX_ID_0;
    }

    init_param.source = convertToInputSourceType(createParams->sourceType);
    init_param.drmmode = inputStreamTypeConvert(createParams->drmMode);
    init_param.dmx_dev_id = createParams->demuxId;
    init_param.event_mask = 0;
    mVideoParaSeted = false;
    mAudioParaSeted = false;

    AmTsPlayer_create(init_param, &mPlayer);

    AmTsPlayer_registerCb(mPlayer, [](void *user_data, am_tsplayer_event *event) {
        static_cast<AmlTsPlayer*>(user_data)->eventCallback(event);
    }, this);
#ifdef HAVE_PACKETIZE_ESTOTS
    int temp_buffer_num   = 100;
    mPacktsBuffer = new AmlMpBuffer(temp_buffer_num * TS_PACKET_SIZE);
    if (AmlMpConfig::instance().mDumpPackts == 1) {
        mPacketsizefd = open("/data/PacketizeEstoTsFile.ts", O_CREAT | O_RDWR, 0666);
    }
#endif
}

int AmlTsPlayer::initCheck() const
{
    return mPlayer != 0 ? 0 : AML_MP_ERROR;
}

AmlTsPlayer::~AmlTsPlayer()
{
    MLOGI("%s:%d", __FUNCTION__, __LINE__);

    if (mPlayer) {
        AmTsPlayer_release(mPlayer);
        mPlayer = AML_MP_INVALID_HANDLE;
    }
    mVideoParaSeted = false;
    mAudioParaSeted = false;

    MLOGI("mBlackOut: %d", mBlackOut);
    if (mBlackOut) {
        if (AmlMpConfig::instance().mTsPlayerNonTunnel == 0) {
#ifdef ANDROID
            if (mNativewindow != nullptr)
                native_window_set_sideband_stream(mNativewindow, nullptr);
#endif
        }
    }
#ifdef HAVE_PACKETIZE_ESTOTS
    if (mPacketsizefd >= 0) {
        close(mPacketsizefd);
    }
#endif
    AmlMpPlayerRoster::instance().signalAmTsPlayerId(-1);
}

int AmlTsPlayer::setANativeWindow(ANativeWindow* nativeWindow)
{
    MLOGI("AmlTsPlayer::setANativeWindow: %p", nativeWindow);
    mNativewindow = nativeWindow;

    int ret = 0;
    if (AmlMpConfig::instance().mTsPlayerNonTunnel) {
        if (AmlMpConfig::instance().mUseVideoTunnel == 0) {
#ifdef ANDROID
#ifndef __ANDROID_VNDK__
            android::Surface* surface = nullptr;
            if (nativeWindow != nullptr) {
                surface = (android::Surface*)nativeWindow;
            }
            MLOGI("setANativeWindow nativeWindow: %p, surface: %p", nativeWindow, surface);
            ret = AmTsPlayer_setSurface(mPlayer, surface);
#endif
#endif
        }
    }

    return ret;
}

int AmlTsPlayer::setVideoParams(const Aml_MP_VideoParams* params) {
    am_tsplayer_result ret;
    if (params->videoCodec == AML_MP_CODEC_UNKNOWN || params->pid == AML_MP_INVALID_PID) {
        MLOGI("amtsplayer invalid video pid or codecid.\n");
        mVideoParaSeted = false;
    } else {
        MLOGI("amtsplayer video params seted.\n");
        mVideoParaSeted = true;
    }
    am_tsplayer_video_params video_params = {convertToVideoCodec(params->videoCodec), params->pid};

    MLOGI("amtsplayer handle:%#zx, video codec:%d, pid:0x%x, secureLevel:%#x", mPlayer, video_params.codectype, video_params.pid, params->secureLevel);
    ret = AmTsPlayer_setVideoParams(mPlayer, &video_params);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }

    Aml_MP_DemuxMemSecLevel secureLevel = params->secureLevel;
    ret = AmTsPlayer_setParams(mPlayer, AM_TSPLAYER_KEY_VIDEO_SECLEVEL, (void*)&secureLevel);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }

    return 0;
}

int AmlTsPlayer::setAudioParams(const Aml_MP_AudioParams* params) {
    am_tsplayer_result ret;
    if (params->audioCodec == AML_MP_CODEC_UNKNOWN || params->pid == AML_MP_INVALID_PID) {
        MLOGI("amtsplayer invalid audio pid or codecid.\n");
        mAudioParaSeted = false;
    } else {
        MLOGI("amtsplayer audio params seted.\n");
        mAudioParaSeted = true;
    }
    am_tsplayer_audio_params audio_params = {convertToAudioCodec(params->audioCodec), params->pid, (int32_t)params->secureLevel};
#ifdef HAVE_PACKETIZE_ESTOTS
    mApid = params->pid;
#endif
    MLOGI("amtsplayer handle:%#zx, audio codec:%d, pid:0x%x, secureLevel:%#x", mPlayer, audio_params.codectype, audio_params.pid, params->secureLevel);
    ret = AmTsPlayer_setAudioParams(mPlayer, &audio_params);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }

    //sample_rate/channel
    am_tsplayer_audio_info audio_extra_params = {params->nSampleRate, params->nChannels, 0/*channel_mask*/, 0/*bitrate*/};
    ret = AmTsPlayer_setParams(mPlayer, AM_TSPLAYER_KEY_SET_ES_AUDIO_EXTRA_PARAM, (void*)&audio_extra_params);
    if (ret != AM_TSPLAYER_OK) {
        MLOGW("set audio extra param failed.");
    }

    Aml_MP_DemuxMemSecLevel secureLevel = params->secureLevel;
    ret = AmTsPlayer_setParams(mPlayer, AM_TSPLAYER_KEY_AUDIO_SECLEVEL, (void*)&secureLevel);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::start() {
    int ret = 0;
    MLOGI("Call start");
    if (mVideoParaSeted)
        ret += startVideoDecoding();
    if (mAudioParaSeted)
        ret += startAudioDecoding();
    //ret += showVideo();

    AmlPlayerBase::start();

    return ret;
}

int AmlTsPlayer::getMediaSyncId() {
    int mediasyncId = -1;
    AmTsPlayer_getSyncInstansNo(mPlayer, &mediasyncId);
    MLOGI("getMediaSyncId:%d\n",mediasyncId);
    return mediasyncId;
}

int AmlTsPlayer::getPlayerId() {
    uint32_t instanceId = -1;
    AmTsPlayer_getInstansNo(mPlayer, &instanceId);
    MLOGI("getPlayerId:%d\n", instanceId);

    return (int)instanceId;
}

int AmlTsPlayer::stop() {
    int ret = 0;

    MLOGI("Call stop");
    AmlPlayerBase::stop();
    if (mVideoParaSeted)
        ret += stopVideoDecoding();
    if (mAudioParaSeted)
        ret += stopAudioDecoding();

    return ret;
}

int AmlTsPlayer::pause() {
    int ret = 0;

    MLOGI("Call pause");
    if (mVideoParaSeted)
        ret += pauseVideoDecoding();
    if (mAudioParaSeted)
        ret += pauseAudioDecoding();
    return ret;
}

int AmlTsPlayer::resume() {
    int ret = 0;

    MLOGI("Call resume");
    if (mVideoParaSeted)
        ret += resumeVideoDecoding();
    if (mAudioParaSeted)
        ret += resumeAudioDecoding();
    return ret;
}

int AmlTsPlayer::flush() {
    //flush need more info, will do in Aml_MP_PlayerImpl
    return AML_MP_ERROR_DEAD_OBJECT;
}

int AmlTsPlayer::setPlaybackRate(float rate){
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;
    MLOGI("setPlaybackRate, rate: %f", rate);
    if (rate == 1.0f) {
        ret = AmTsPlayer_stopFast(mPlayer);
    } else {
        ret = AmTsPlayer_startFast(mPlayer, rate);
    }
    if (ret != AM_TSPLAYER_OK)
        return -1;
    return 0;
}

int AmlTsPlayer::getPlaybackRate(float* rate) {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;
    //TODO: Now media_hal compile not correct, return playback rate in aml_mp,
    //      need get rate info from decoder after compile correct
#if 0
    ret = AmTsPlayer_getFastRate(mPlayer, rate);
#endif
    if (ret != AM_TSPLAYER_OK) {
        return AML_MP_ERROR;
    }
    MLOGI("getPlaybackRate, rate: %f", *rate);
    return AML_MP_OK;
}

int AmlTsPlayer::switchAudioTrack(const Aml_MP_AudioParams* params){
    //switchAudioTrack need more info, will do in Aml_MP_PlayerImpl
    AML_MP_UNUSED(params);
    return AML_MP_ERROR_DEAD_OBJECT;
}

int AmlTsPlayer::writeData(const uint8_t* buffer, size_t size) {
    //AML_MP_TRACE(10);
    am_tsplayer_result ret;
    am_tsplayer_input_buffer buf ={init_param.drmmode, (void*)buffer, (int32_t)size};
    ret = AmTsPlayer_writeData(mPlayer, &buf, kRwTimeout);
    //MLOGI("writedata, buffer:%p, size:%d, ret:%d", buffer, size, ret);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return size;
}

int AmlTsPlayer::writeEsData(Aml_MP_StreamType type, const uint8_t* buffer, size_t size, int64_t pts)
{
    int ret = -1;
    if (type == AML_MP_STREAM_TYPE_AUDIO && mAudioEsDataFeedThread) {
        ret = mAudioEsDataFeedThread->writeEsData(buffer, size, pts);
    } else {
        ret = writeEsData_l(type, buffer, size, pts);
    }
    return ret;
}

int AmlTsPlayer::writeEsData_l(Aml_MP_StreamType type, const uint8_t* buffer, size_t size, int64_t pts)
{
#ifdef HAVE_PACKETIZE_ESTOTS
    sptr<AmlMpBuffer> tsPackets;
    int ret;
    ret = packetize(1, (char *)buffer, size, &tsPackets, 0, NULL, 0, 2, pts);
    //MLOGI("writeEsdata, buffer:%p, size:%d, ret:%d", buffer, size, ret);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
#else
    if (type == AML_MP_STREAM_TYPE_SUBTITLE) {
        return AmlPlayerBase::writeEsData(type, buffer, size, pts);
    }

    am_tsplayer_result ret;
    am_tsplayer_input_frame_buffer buf;
    buf.buf_type = init_param.drmmode;
    buf.buf_data = (void *)buffer;
    buf.buf_size = size;
    buf.pts = pts;
    buf.isvideo = 0;
    if (type == AML_MP_STREAM_TYPE_VIDEO) {
        buf.isvideo = 1;
    }

    ret = AmTsPlayer_writeFrameData(mPlayer, &buf, kRwTimeout);

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return size;
#endif
}

int AmlTsPlayer::packetize(
        bool isAudio,
        const char *buffer_add,
        size_t buffer_size,
        sptr<AmlMpBuffer> *packets,
        uint32_t flags,
        const uint8_t *PES_private_data, size_t PES_private_data_len,
        size_t numStuffingBytes,
        int64_t timeUs)
{
#define RANDOM_VALID_AUDIO_STREAM_PID 0x102
#define AUDIO_STREAM_ID 0xc0
#define TS_PACKET_HEADER_SIZE 4
#define PES_PACKET_LENGTH_MAX 65536
    AML_MP_UNUSED(flags);

    int32_t stream_pid = RANDOM_VALID_AUDIO_STREAM_PID;
    int32_t stream_id = 0x00;

    if (isAudio) {
        stream_id = AUDIO_STREAM_ID;
    } else {
        MLOGE("packetize error! only support audio es data");
        return -1;
    }

    if (mApid != AML_MP_INVALID_PID) {
        stream_pid = mApid;
    }

    packets->clear();
    int ret = 0;
    size_t PES_packet_length = buffer_size + 8 + numStuffingBytes;
    if (PES_private_data_len > 0) {
        PES_packet_length += PES_private_data_len + 1;
    }

    size_t numTSPackets = 1;
    {
        // Make sure the PES header fits into a single TS packet:
        size_t PES_header_size = 14 + numStuffingBytes;
        if (PES_private_data_len > 0) {
            PES_header_size += PES_private_data_len + 1;
        }

        size_t sizeAvailableForPayload = TS_PACKET_SIZE - TS_PACKET_HEADER_SIZE - PES_header_size;
        size_t numBytesOfPayload = buffer_size;
        if (numBytesOfPayload > sizeAvailableForPayload) {
            numBytesOfPayload = sizeAvailableForPayload;
        }

        // size_t numPaddingBytes = sizeAvailableForPayload - numBytesOfPayload;
        size_t numBytesOfPayloadRemaining = buffer_size - numBytesOfPayload;
        // This is how many bytes of payload each subsequent TS packet
        // can contain at most.
        sizeAvailableForPayload = TS_PACKET_SIZE - TS_PACKET_HEADER_SIZE;
        size_t sizeAvailableForAlignedPayload = sizeAvailableForPayload;

        /*divide the PayloadRemaining and calculate how many ts packet can contain it*/
        size_t numFullTSPackets = numBytesOfPayloadRemaining / sizeAvailableForAlignedPayload;
        numTSPackets += numFullTSPackets;
        numBytesOfPayloadRemaining -= numFullTSPackets * sizeAvailableForAlignedPayload;

        // numBytesOfPayloadRemaining < sizeAvailableForAlignedPayload
        if (numFullTSPackets == 0 && numBytesOfPayloadRemaining > 0) {
            // There wasn't enough payload left to form a full aligned payload,
            // the last packet doesn't have to be aligned.
            ++numTSPackets;
        } else if (numFullTSPackets > 0 && numBytesOfPayloadRemaining
                   + sizeAvailableForAlignedPayload > sizeAvailableForPayload) {
            // The last packet emitted had a full aligned payload and together
            // with the bytes remaining does exceed the unaligned payload
            // size, so we need another packet.
            ++numTSPackets;
        }
    }
    /*malloc spaces for those ts packets*/
    if (numTSPackets * TS_PACKET_SIZE > mPacktsBuffer->capacity()) {
        mPacktsBuffer = new AmlMpBuffer(numTSPackets * TS_PACKET_SIZE);
    }
    //MLOGI("second mPacktsBuffer=%p,numTSPackets=%d,mPacktsBuffer.size=%d,mPacktsBuffer.capacity=%d\n",mPacktsBuffer.get(),numTSPackets,mPacktsBuffer->size(),mPacktsBuffer->capacity());
    uint8_t *packetDataStart = mPacktsBuffer->data();
    uint64_t PTS = (timeUs * 9ll) / 100ll;
    if (PES_packet_length >= PES_PACKET_LENGTH_MAX) {
        // This really should only happen for video.
        // It's valid to set this to 0 for video according to the specs.
        PES_packet_length = 0;
    }

    size_t sizeAvailableForPayload = TS_PACKET_SIZE - TS_PACKET_HEADER_SIZE - 14 - numStuffingBytes;
    if (PES_private_data_len > 0) {
        sizeAvailableForPayload -= PES_private_data_len + 1;
    }

    size_t copy = buffer_size;
    if (copy > sizeAvailableForPayload) {
        copy = sizeAvailableForPayload;
    }

    size_t numPaddingBytes = sizeAvailableForPayload - copy;
    uint8_t *ptr = packetDataStart;
    /* prepare packet header */
    *ptr++ = 0x47;
    *ptr++ = 0x40 | (stream_pid >> 8);
    *ptr++ = stream_pid & 0xff;
    *ptr++ = (numPaddingBytes > 0 ? 0x30 : 0x10) | incrementContinuityCounter(isAudio);

    if (numPaddingBytes > 0) {
        *ptr++ = numPaddingBytes - 1;
        if (numPaddingBytes >= 2) {
            *ptr++ = 0x00;
            memset(ptr, 0xff, numPaddingBytes - 2);
            ptr += numPaddingBytes - 2;
        }
    }
    /* write PES header */
    *ptr++ = 0x00;
    *ptr++ = 0x00;
    *ptr++ = 0x01;
    *ptr++ = stream_id;
    *ptr++ = PES_packet_length >> 8;
    *ptr++ = PES_packet_length & 0xff;
    *ptr++ = 0x84;
    *ptr++ = (PES_private_data_len > 0) ? 0x81 : 0x80;

    size_t headerLength = 0x05 + numStuffingBytes;
    if (PES_private_data_len > 0) {
        headerLength += 1 + PES_private_data_len;
    }
    /***write pts***/
    *ptr++ = headerLength;
    *ptr++ = 0x20 | (((PTS >> 30) & 7) << 1) | 1;
    *ptr++ = (PTS >> 22) & 0xff;
    *ptr++ = (((PTS >> 15) & 0x7f) << 1) | 1;
    *ptr++ = (PTS >> 7) & 0xff;
    *ptr++ = ((PTS & 0x7f) << 1) | 1;

    if (PES_private_data_len > 0) {
        *ptr++ = 0x8e;// PES_private_data_flag, reserved.
        memcpy(ptr, PES_private_data, PES_private_data_len);
        ptr += PES_private_data_len;
    }

    for (size_t i = 0; i < numStuffingBytes; ++i) {
        *ptr++ = 0xff;
    }
    /*copy es data then pack next ts packet*/
    memcpy(ptr, buffer_add, copy);
    ptr += copy;
    packetDataStart += TS_PACKET_SIZE;
    size_t offset = copy;
    while (offset < buffer_size) {
        size_t sizeAvailableForPayload = TS_PACKET_SIZE - TS_PACKET_HEADER_SIZE;
        size_t copy = buffer_size - offset;
        if (copy > sizeAvailableForPayload) {
            copy = sizeAvailableForPayload;
        }

        size_t numPaddingBytes = sizeAvailableForPayload - copy;
        uint8_t *ptr = packetDataStart;
        *ptr++ = 0x47;
        *ptr++ = 0x00 | (stream_pid >> 8);
        *ptr++ = stream_pid & 0xff;
        *ptr++ = (numPaddingBytes > 0 ? 0x30 : 0x10) | incrementContinuityCounter(isAudio);

        if (numPaddingBytes > 0) {
            *ptr++ = numPaddingBytes - 1;
            if (numPaddingBytes >= 2) {
                *ptr++ = 0x00;
                memset(ptr, 0xff, numPaddingBytes - 2);
                ptr += numPaddingBytes - 2;
            }
        }

        memcpy(ptr, buffer_add + offset, copy);
        ptr += copy;
        offset += copy;
        packetDataStart += TS_PACKET_SIZE;
    }

    *packets = mPacktsBuffer;
    if (mPacketsizefd >= 0) {
        write(mPacketsizefd, mPacktsBuffer->data(), numTSPackets * TS_PACKET_SIZE);
        //MLOGE("[%s %d] PacketizeEstoTsFile.ts size:%d", __FUNCTION__, __LINE__, numTSPackets * TS_PACKET_SIZE);
    }
    ret = writeData(mPacktsBuffer->data(),numTSPackets * TS_PACKET_SIZE);
    if (ret != AM_TSPLAYER_OK) {
        packets->clear();
        return -1;
    }
    return AM_TSPLAYER_OK;
}

int AmlTsPlayer::incrementContinuityCounter(int isAudio)
{
    unsigned prevCounter = 0;
    if (isAudio) {
        prevCounter = mAudioContinuityCounter;
        if (++mAudioContinuityCounter == 16) {
            mAudioContinuityCounter = 0;
        }
    }
    return prevCounter;
}

int AmlTsPlayer::getCurrentPts(Aml_MP_StreamType type, int64_t* pts) {
    am_tsplayer_result ret;

    switch (type) {
        case AML_MP_STREAM_TYPE_VIDEO:
        case AML_MP_STREAM_TYPE_AUDIO:
        case AML_MP_STREAM_TYPE_AD:
        case AML_MP_STREAM_TYPE_SUBTITLE:
        {
            ret = AmTsPlayer_getPts(mPlayer, convertToTsplayerStreamType(type), (uint64_t*)pts);
            break;
        }
        case AML_MP_STREAM_TYPE_PCR:
        case AML_MP_STREAM_TYPE_STC:
        {
            ret = AmTsPlayer_getMediaTime(mPlayer, convertToTsplayerMediaTimeType(type), TS_UNIT_PTS, (uint64_t*)pts);
            break;
        }
        default:
        {
            ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;
            break;
        }
    }

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::getFirstPts(Aml_MP_StreamType type, int64_t* pts) {
    am_tsplayer_result ret;

    ret = AmTsPlayer_getFirstPts(mPlayer, convertToTsplayerStreamType(type), (uint64_t*)pts);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::getBufferStat(Aml_MP_BufferStat* bufferStat) {
    am_tsplayer_result ret;
    am_tsplayer_buffer_stat buffer_stat{0, 0, 0};

    ret = AmTsPlayer_getBufferStat(mPlayer, TS_STREAM_AUDIO, &buffer_stat);
    if (ret != AM_TSPLAYER_OK) {
        MLOGE("Get audio buffer error, ret = %d", ret);
        return -1;
    }
    bufferStat->audioBuffer.size = buffer_stat.size;
    bufferStat->audioBuffer.dataLen = buffer_stat.data_len;

    ret = AmTsPlayer_getBufferStat(mPlayer, TS_STREAM_VIDEO, &buffer_stat);
    bufferStat->videoBuffer.size = buffer_stat.size;
    bufferStat->videoBuffer.dataLen = buffer_stat.data_len;
    if (ret != AM_TSPLAYER_OK) {
        MLOGE("Get video buffer error, ret = %d", ret);
        return -1;
    }

    int64_t cacheMs = 0;
    AmTsPlayer_getDelayTime(mPlayer, &cacheMs);
    bufferStat->audioBuffer.bufferedMs = cacheMs;
    bufferStat->videoBuffer.bufferedMs = cacheMs;

    return 0;
}

int AmlTsPlayer::setVideoWindow(int x, int y, int width, int height) {
    am_tsplayer_result ret;

    MLOGI("setVideoWindow, x: %d, y: %d, width: %d, height: %d", x, y, width, height);
    ret = AmTsPlayer_setVideoWindow(mPlayer, x, y, width, height);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::setVolume(float volume) {
    am_tsplayer_result ret;
    int32_t tsplayer_volume = volume;

    MLOGI("setVolume, tsplayer_volume: %d", tsplayer_volume);
    ret = AmTsPlayer_setAudioVolume(mPlayer, tsplayer_volume);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::getVolume(float* volume) {
    am_tsplayer_result ret;
    int32_t tsplayer_volume;

    ret = AmTsPlayer_getAudioVolume(mPlayer, &tsplayer_volume);
    MLOGI("getVolume volume: %d, ret: %d", tsplayer_volume, ret);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    *volume = tsplayer_volume;
    return 0;
}

int AmlTsPlayer::showVideo() {
    am_tsplayer_result ret;
    ret = AmTsPlayer_showVideo(mPlayer);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::hideVideo() {
    am_tsplayer_result ret;
    ret = AmTsPlayer_hideVideo(mPlayer);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::setParameter(Aml_MP_PlayerParameterKey key, void* parameter) {
    int ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;

    switch (key) {
        case AML_MP_PLAYER_PARAMETER_VIDEO_DISPLAY_MODE:
            MLOGI("setVideoDisplayMode: %d", *(Aml_MP_VideoDisplayMode*)parameter);
            ret = AmTsPlayer_setVideoMatchMode(mPlayer, convertToTsPlayerVideoMatchMode(*(Aml_MP_VideoDisplayMode*)parameter));
            break;

        case AML_MP_PLAYER_PARAMETER_BLACK_OUT:
        {
            int blackOut = *(bool_t*)parameter;
            MLOGI("setVideoBlackOut: %d", blackOut);
            ret = AmTsPlayer_setVideoBlackOut(mPlayer, blackOut);
            mBlackOut = blackOut;
            break;
        }

        case AML_MP_PLAYER_PARAMETER_VIDEO_DECODE_MODE:
            MLOGI("setVideoDecodeMode: %s", mpVideoDecodeMode2Str(*(Aml_MP_VideoDecodeMode*)parameter));
            ret = AmTsPlayer_setTrickMode(mPlayer, convertToTsplayerVideoTrickMode(*(Aml_MP_VideoDecodeMode*)parameter));
            break;

        case AML_MP_PLAYER_PARAMETER_VIDEO_PTS_OFFSET:
            break;

        case AML_MP_PLAYER_PARAMETER_AUDIO_OUTPUT_MODE:
            MLOGI("setAudioOutputMode: %d", *(Aml_MP_AudioOutputMode*)parameter);
            ret = AmTsPlayer_setAudioOutMode(mPlayer, convertToTsPlayerAudioOutMode(*(Aml_MP_AudioOutputMode*)parameter));
            break;

        case AML_MP_PLAYER_PARAMETER_AUDIO_OUTPUT_DEVICE:
            break;

        case AML_MP_PLAYER_PARAMETER_AUDIO_PTS_OFFSET:
            break;

        case AML_MP_PLAYER_PARAMETER_AUDIO_BALANCE:
            MLOGI("setAudioBalance: %d", *(Aml_MP_AudioBalance*)parameter);
            ret = AmTsPlayer_setAudioStereoMode(mPlayer, convertToTsPlayerAudioStereoMode(*(Aml_MP_AudioBalance*)parameter));
            break;

        case AML_MP_PLAYER_PARAMETER_AUDIO_MUTE:
        {
            bool mute = *(bool*)parameter;
            MLOGI("setAudioMute: %d", mute);
            ret =AmTsPlayer_setAudioMute(mPlayer, mute, mute);
            break;
        }

        case AML_MP_PLAYER_PARAMETER_NETWORK_JITTER:
            break;

        case AML_MP_PLAYER_PARAMETER_AD_STATE:
        {
            int isEnable = *(int*)parameter;
            MLOGI("setADState: %d", isEnable);
            if (isEnable)
                ret = AmTsPlayer_enableADMix(mPlayer);
            else
                ret = AmTsPlayer_disableADMix(mPlayer);
        }
        break;

        case AML_MP_PLAYER_PARAMETER_AD_MIX_LEVEL:
        {
            Aml_MP_ADVolume* ADVolume = (Aml_MP_ADVolume*)parameter;
            MLOGI("setADMixLevel: %d/%d", ADVolume->masterVolume, ADVolume->slaveVolume);
            ret = AmTsPlayer_setADMixLevel(mPlayer, ADVolume->masterVolume, ADVolume->slaveVolume);
        }
        break;

        case AML_MP_PLAYER_PARAMETER_WORK_MODE:
            MLOGI("setWorkMode: %s", mpPlayerWorkMode2Str(*(Aml_MP_PlayerWorkMode*)(parameter)));
            ret = AmTsPlayer_setWorkMode(mPlayer, *(am_tsplayer_work_mode*)(parameter));
            break;

        case AML_MP_PLAYER_PARAMETER_TELETEXT_CONTROL:
        {
            ret = AmlPlayerBase::setParameter(key, parameter);
            break;
        }

        case AML_MP_PLAYER_PARAMETER_AUDIO_BLOCK_ALIGN:
        {
            char block_align[50] = {0};
            sprintf(block_align, "adpcm_block_size=%d", *(int*)parameter);
            MLOGI("block_align: %s", block_align);
            //Reusing the key "AM_TSPLAYER_KEY_SET_WMA_DESCR".
            ret = AmTsPlayer_setParams(mPlayer, AM_TSPLAYER_KEY_SET_WMA_DESCR, (void*)block_align);

            break;
        }

        case AML_MP_PLAYER_PARAMETER_VIDEO_TUNNEL_ID:
        {
            mVideoTunnelId = *(int*)parameter;
            MLOGI("setVideoTunnelID: %d", mVideoTunnelId);
            ret = AmTsPlayer_setSurface(mPlayer, &mVideoTunnelId);
            break;
        }

        case AML_MP_PLAYER_PARAMETER_SURFACE_HANDLE:
        {
#ifdef ANDROID
            if (AmlMpConfig::instance().mUseVideoTunnel) {
                // this is video tunnel id, must be a member variable address
                // parameter variable stores videoTunnel ID value instead if its address.
                mVideoTunnelId = (int)(intptr_t)parameter;
                MLOGI("set videoTunnelId: %d", mVideoTunnelId);
                ret = AmTsPlayer_setSurface(mPlayer, &mVideoTunnelId);
            } else
#endif
            {
                void* surface = parameter;
                ret = AmTsPlayer_setSurface(mPlayer, surface);
            }

            break;
        }

        case AML_MP_PLAYER_PARAMETER_AUDIO_PRESENTATION_ID:
        {
            int para = *(int*)parameter;
            MLOGI("setPresentationId: %d", para);
            if (para >= 0) {
                ret = AmTsPlayer_setParams(mPlayer, AM_TSPLAYER_KEY_AUDIO_PRESENTATION_ID, parameter);
            }
            break;
        }

        case AML_MP_PLAYER_PARAMETER_USE_TIF:
        {
#ifdef ANDROID //yocto mediahal hasn't define this function
            am_tsplayer_audio_patch_manage_mode audioPatchManageMode = AUDIO_PATCH_MANAGE_AUTO;
            int para = *(bool*)parameter;
            MLOGI("setUseTif: %d", para);
            if (para != -1) {
                audioPatchManageMode = para ? AUDIO_PATCH_MANAGE_FORCE_DISABLE : AUDIO_PATCH_MANAGE_FORCE_ENABLE;
            }
            ret = AmTsPlayer_setParams(mPlayer, AM_TSPLAYER_KEY_SET_AUDIO_PATCH_MANAGE_MODE, (void*)&audioPatchManageMode);
#endif
            break;
        }

        case AML_MP_PLAYER_PARAMETER_SPDIF_PROTECTION:
        {
            int para = *(int*)parameter;
            MLOGI("SetSPDIFProtection: %d", para);
            if (para != -1) {
                ret = AmTsPlayer_setParams(mPlayer, AM_TSPLAYER_KEY_SET_SPDIF_STATUS, parameter);
            }
            break;
        }

        case AML_MP_PLAYER_PARAMETER_VIDEO_CROP:
        {
            Aml_MP_Rect videoCrop = *(Aml_MP_Rect*)parameter;
            MLOGI("setVideoCrop: [%d, %d, %d, %d]", videoCrop.left, videoCrop.top, videoCrop.right, videoCrop.bottom);
            ret = AmTsPlayer_setVideoCrop(mPlayer, videoCrop.left, videoCrop.top, videoCrop.right, videoCrop.bottom);
        }
        break;

        case AML_MP_PLAYER_PARAMETER_VIDEO_ERROR_RECOVERY_MODE:
        {
            MLOGI("setVideoErrorRecoveryMode: %s", mpVideoErrorRecoveryMode2Str(*(Aml_MP_VideoErrorRecoveryMode*)parameter));
            int recoveryMode = convertToCodecRecoveryMode(*(Aml_MP_VideoErrorRecoveryMode*)parameter);
            ret = AmTsPlayer_setParams(mPlayer, AM_TSPLAYER_KEY_SET_VIDEO_RECOVERY_MODE, &recoveryMode);
        }
        break;

        case AML_MP_PLAYER_PARAMETER_AUDIO_LANGUAGE:
        {
            am_tsplayer_audio_lang audioLang;
            convertToTsPlayerAudioLanguage(&audioLang, (Aml_MP_AudioLanguage*)parameter);
            ret = AmTsPlayer_setParams(mPlayer, AM_TSPLAYER_KEY_SET_AUDIO_LANG, &audioLang);
        }
        break;

        default:
            ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;
    }
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::getParameter(Aml_MP_PlayerParameterKey key, void* parameter) {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;

    RETURN_IF(-1, parameter == nullptr);

    switch (key) {
        case AML_MP_PLAYER_PARAMETER_AUDIO_MUTE:
        {
            bool_t analog_unmute, digital_unmute;
            ret = AmTsPlayer_getAudioMute(mPlayer, &analog_unmute, &digital_unmute);
            *(bool*)parameter = digital_unmute;
            break;
        }

        case AML_MP_PLAYER_PARAMETER_VIDEO_INFO:
            am_tsplayer_video_info videoInfo;
            ret = AmTsPlayer_getVideoInfo(mPlayer, &videoInfo);
            convertToMpVideoInfo((Aml_MP_VideoInfo*)parameter, &videoInfo);
            break;
        case AML_MP_PLAYER_PARAMETER_VIDEO_DECODE_STAT:
            ret = AmTsPlayer_getVideoStat(mPlayer, (am_tsplayer_vdec_stat*)parameter);
            break;
        case AML_MP_PLAYER_PARAMETER_AUDIO_INFO:
            ret = AmTsPlayer_getAudioInfo(mPlayer, (am_tsplayer_audio_info*)parameter);
            break;
        case AML_MP_PLAYER_PARAMETER_AUDIO_DECODE_STAT:
            ret = AmTsPlayer_getAudioStat(mPlayer, (am_tsplayer_adec_stat*) parameter);
            break;
        case AML_MP_PLAYER_PARAMETER_AD_INFO:
            ret = AmTsPlayer_getADInfo(mPlayer, (am_tsplayer_audio_info*)parameter);
            break;
        case AML_MP_PLAYER_PARAMETER_AD_DECODE_STAT:
            ret = AmTsPlayer_getADStat(mPlayer, (am_tsplayer_adec_stat*)parameter);
            break;

        case AML_MP_PLAYER_PARAMETER_INSTANCE_ID:
            ret = AmTsPlayer_getInstansNo(mPlayer, (uint32_t*)parameter);
            break;
        case AML_MP_PLAYER_PARAMETER_SYNC_ID:
            ret = AmTsPlayer_getSyncInstansNo(mPlayer, (int32_t*)parameter);
            break;
        case AML_MP_PLAYER_PARAMETER_VIDEO_ERROR_RECOVERY_MODE:
        {
            //TODO: tsplayer not have api now, need add api later
            ret = AM_TSPLAYER_ERROR_INVALID_OPERATION;
        }
        break;

        case AML_MP_PLAYER_PARAMETER_AV_INFO_JSON: {
            Aml_MP_AvInfo *mpAvInfo = (Aml_MP_AvInfo*)parameter;
            am_tsplayer_state_t tsAvInfo;
            bool_t hasVideo = false;
            bool_t hasAudio = false;
            tsAvInfo.data = mpAvInfo->data;
            tsAvInfo.data_len = mpAvInfo->dataLength;
            if (mpAvInfo->streamTypeMask & AML_MP_STREAM_TYPE_MASK_VIDEO) {
                hasVideo = true;
            }
            if (mpAvInfo->streamTypeMask & AML_MP_STREAM_TYPE_MASK_AUDIO) {
                hasAudio = true;
            }

#ifdef ANDROID //yocto mediahal hasn't define av_flag
            if (hasVideo && hasAudio) {
                tsAvInfo.av_flag = (am_tsplayer_av_info_state)0;
            } else if (hasAudio) {
                tsAvInfo.av_flag = (am_tsplayer_av_info_state)1;
            } else if (hasVideo) {
                tsAvInfo.av_flag = (am_tsplayer_av_info_state)2;
            }
#endif
            ret = AmTsPlayer_getState(mPlayer, &tsAvInfo);
            mpAvInfo->actualLength = tsAvInfo.actual_len;
            break;
        }

        case AML_MP_PLAYER_PARAMETER_AD_MIX_LEVEL:
        {
            Aml_MP_ADVolume* ADVolume = (Aml_MP_ADVolume*)parameter;
            ret = AmTsPlayer_getADMixLevel(mPlayer, (int32_t*)(&ADVolume->masterVolume), (int32_t*)(&ADVolume->slaveVolume));
            //MLOGI("trace getParameter, AML_MP_PLAYER_PARAMETER_AD_MIX_LEVEL, value is master %d, slave %d %p, %p", ADVolume->masterVolume, ADVolume->slaveVolume, ADVolume, parameter);
            break;
        }

        case AML_MP_PLAYER_PARAMETER_TSPLAYER_HANDLE:
        {
            am_tsplayer_handle* handle = (am_tsplayer_handle*)parameter;
            *handle = mPlayer;
            ret = AM_TSPLAYER_OK;
        }
        break;

        case AML_MP_PLAYER_PARAMETER_AUDIO_PRESENTATION_ID:
        {
            ret = AmTsPlayer_getParams(mPlayer, AM_TSPLAYER_KEY_AUDIO_PRESENTATION_ID, parameter);
            break;
        }

        default:
            ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;
    }

    MLOGD("Call getParameter, key is %s, ret: %d", mpPlayerParameterKey2Str(key), ret);
    switch (ret) {
        case AM_TSPLAYER_OK:
            return AML_MP_OK;
        case AM_TSPLAYER_ERROR_INVALID_PARAMS:
            return AML_MP_ERROR_BAD_VALUE;
        case AM_TSPLAYER_ERROR_INVALID_OPERATION:
            return AML_MP_ERROR_INVALID_OPERATION;
        default:
            return AML_MP_ERROR;
    }

}

int AmlTsPlayer::setAVSyncSource(Aml_MP_AVSyncSource syncSource)
{
    am_tsplayer_result ret = AM_TSPLAYER_OK;

    MLOGI("setsyncmode, syncSource %s!!!", mpAVSyncSource2Str(syncSource));
    MLOGI("converted syncSource is: %d", convertToAVSyncSourceType(syncSource));
    ret = AmTsPlayer_setSyncMode(mPlayer, convertToAVSyncSourceType(syncSource));
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::setPcrPid(int pid) {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;

    ret = AmTsPlayer_setPcrPid(mPlayer, (uint32_t)pid);

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::startVideoDecoding() {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;

    ret = AmTsPlayer_startVideoDecoding(mPlayer);

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::stopVideoDecoding() {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;

    ret = AmTsPlayer_stopVideoDecoding(mPlayer);

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;

}

int AmlTsPlayer::pauseVideoDecoding() {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;

    ret = AmTsPlayer_pauseVideoDecoding(mPlayer);

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::resumeVideoDecoding() {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;

    ret = AmTsPlayer_resumeVideoDecoding(mPlayer);

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::startAudioDecoding() {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;
    ret = AmTsPlayer_startAudioDecoding(mPlayer);

    if (!mAudioEsDataFeedThread) {
        mAudioEsDataFeedThread = new AudioEsDataFeedThread(this);
        mAudioEsDataFeedThread->start();
    }
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::stopAudioDecoding() {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;

    if (mAudioEsDataFeedThread) {
        mAudioEsDataFeedThread->stop();
        mAudioEsDataFeedThread.clear();
    }

    ret = AmTsPlayer_stopAudioDecoding(mPlayer);

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::startADDecoding()
{
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;

    ret = AmTsPlayer_startAudioDecoding(mPlayer);

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::stopADDecoding()
{
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;

    ret = AmTsPlayer_stopAudioDecoding(mPlayer);

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::pauseAudioDecoding() {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;
    if (mAudioEsDataFeedThread) {
        mAudioEsDataFeedThread->pause();
    }
    ret = AmTsPlayer_pauseAudioDecoding(mPlayer);

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::resumeAudioDecoding() {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;

    ret = AmTsPlayer_resumeAudioDecoding(mPlayer);
    if (mAudioEsDataFeedThread) {
        mAudioEsDataFeedThread->resume();
    }

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}


int AmlTsPlayer::setADParams(const Aml_MP_AudioParams* params, bool enableMix) {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;
    am_tsplayer_audio_params audioParams;

    audioParams.pid = (int32_t)(params->pid);
    audioParams.codectype = convertToAudioCodec(params->audioCodec);
    audioParams.seclevel = (int32_t)params->secureLevel;

    ret = AmTsPlayer_setADParams(mPlayer, &audioParams);
    if (enableMix) {
        AmTsPlayer_enableADMix(mPlayer);
    } else {
        AmTsPlayer_disableADMix(mPlayer);
    }

    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::setADVolume(float volume) {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;
    int32_t tsplayer_advolume = volume;

    MLOGI("setADVolume, tsplayer_advolume: %d", tsplayer_advolume);

    ret = AmTsPlayer_setADVolume(mPlayer, tsplayer_advolume);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    return 0;
}

int AmlTsPlayer::getADVolume(float* volume) {
    am_tsplayer_result ret = AM_TSPLAYER_ERROR_INVALID_PARAMS;
    int32_t tsplayer_advolume = 0;

    if (volume == NULL) {
        return -1;
    }

    ret = AmTsPlayer_getADVolume(mPlayer, &tsplayer_advolume);

    MLOGI("getADVolume tsplayer_advolume: %d, ret: %d", tsplayer_advolume, ret);
    if (ret != AM_TSPLAYER_OK) {
        return -1;
    }
    *volume = tsplayer_advolume;
    return 0;
}

void AmlTsPlayer::eventCallback(am_tsplayer_event* event)
{
    switch (event->type) {
    case AM_TSPLAYER_EVENT_TYPE_VIDEO_CHANGED:
    {
        MLOGE("[evt] AML_MP_PLAYER_EVENT_VIDEO_CHANGED");

        Aml_MP_PlayerEventVideoFormat videoFormatEvent;
        videoFormatEvent.frame_width = event->event.video_format.frame_width;
        videoFormatEvent.frame_height = event->event.video_format.frame_height;
        videoFormatEvent.frame_rate = event->event.video_format.frame_rate;
        videoFormatEvent.frame_aspectratio = event->event.video_format.frame_aspectratio;

        notifyListener(AML_MP_PLAYER_EVENT_VIDEO_CHANGED, (int64_t)&videoFormatEvent);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_AUDIO_CHANGED:
    {
        MLOGE("[evt] AM_TSPLAYER_EVENT_TYPE_AUDIO_CHANGED\n");
        Aml_MP_PlayerEventAudioFormat audioFormatEvent;
        convertToMpPlayerEventAudioFormat(&audioFormatEvent, &(event->event.audio_format));
        notifyListener(AML_MP_PLAYER_EVENT_AUDIO_CHANGED, (int64_t)&audioFormatEvent);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_DECODE_FIRST_FRAME_VIDEO:
    {
        MLOGE("[evt] AM_TSPLAYER_EVENT_TYPE_DECODE_FIRST_FRAME_VIDEO\n");
        notifyListener(AML_MP_PLAYER_EVENT_VIDEO_DECODE_FIRST_FRAME);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_DECODE_FIRST_FRAME_AUDIO:
    {
        MLOGE("[evt] AM_TSPLAYER_EVENT_TYPE_DECODE_FIRST_FRAME_AUDIO\n");
        notifyListener(AML_MP_PLAYER_EVENT_AUDIO_DECODE_FIRST_FRAME);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_FIRST_FRAME:
        MLOGE("[evt] AM_TSPLAYER_EVENT_TYPE_FIRST_FRAME\n");

        notifyListener(AML_MP_PLAYER_EVENT_FIRST_FRAME);
        break;

    case AM_TSPLAYER_EVENT_TYPE_AV_SYNC_DONE:
        MLOGE("[evt] AML_MP_PLAYER_EVENT_AV_SYNC_DONE");

        notifyListener(AML_MP_PLAYER_EVENT_AV_SYNC_DONE);
        break;

    case AM_TSPLAYER_EVENT_TYPE_INPUT_VIDEO_BUFFER_DONE:
        notifyListener(AML_MP_PLAYER_EVENT_VIDEO_INPUT_BUFFER_DONE, (int64_t)event->event.bufptr);
        break;

    case AM_TSPLAYER_EVENT_TYPE_INPUT_AUDIO_BUFFER_DONE:
        notifyListener(AML_MP_PLAYER_EVENT_AUDIO_INPUT_BUFFER_DONE, (int64_t)event->event.bufptr);
        break;

    case AM_TSPLAYER_EVENT_TYPE_DATA_LOSS:
        notifyListener(AML_MP_PLAYER_EVENT_DATA_LOSS);
        break;

    case AM_TSPLAYER_EVENT_TYPE_DATA_RESUME:
        notifyListener(AML_MP_PLAYER_EVENT_DATA_RESUME);
        break;

    case AM_TSPLAYER_EVENT_TYPE_SCRAMBLING:
    {
        Aml_MP_PlayerEventScrambling scrambling;
        scrambling.scramling = 1;
        scrambling.type = convertToAmlMPStreamType(event->event.scramling.stream_type);

        notifyListener(AML_MP_PLAYER_EVENT_SCRAMBLING, (int64_t)&scrambling);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_USERDATA_AFD:
    {
        Aml_MP_PlayerEventMpegUserData userData;
        userData.data = event->event.mpeg_user_data.data;
        userData.len = event->event.mpeg_user_data.len;

        notifyListener(AML_MP_PLAYER_EVENT_USERDATA_AFD, (int64_t)&userData);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_USERDATA_CC:
    {
        Aml_MP_PlayerEventMpegUserData userData;
        userData.data = event->event.mpeg_user_data.data;
        userData.len = event->event.mpeg_user_data.len;

        notifyListener(AML_MP_PLAYER_EVENT_USERDATA_CC, (int64_t)&userData);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_VIDEO_OVERFLOW:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_VIDEO_OVERFLOW\n");
        uint32_t video_overflow_num;
        video_overflow_num = event->event.av_flow_cnt.video_overflow_num;
        notifyListener(AML_MP_PLAYER_EVENT_VIDEO_OVERFLOW, (int64_t)&video_overflow_num);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_VIDEO_UNDERFLOW:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_VIDEO_UNDERFLOW\n");
        uint32_t video_underflow_num;
        video_underflow_num = event->event.av_flow_cnt.video_underflow_num;
        notifyListener(AML_MP_PLAYER_EVENT_VIDEO_UNDERFLOW, (int64_t)&video_underflow_num);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_AUDIO_OVERFLOW:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_AUDIO_OVERFLOW\n");
        uint32_t audio_overflow_num;
        audio_overflow_num = event->event.av_flow_cnt.audio_overflow_num;
        notifyListener(AML_MP_PLAYER_EVENT_AUDIO_OVERFLOW, (int64_t)&audio_overflow_num);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_AUDIO_UNDERFLOW:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_AUDIO_UNDERFLOW\n");
        uint32_t audio_underflow_num;
        audio_underflow_num = event->event.av_flow_cnt.audio_underflow_num;
        notifyListener(AML_MP_PLAYER_EVENT_AUDIO_UNDERFLOW, (int64_t)&audio_underflow_num);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_VIDEO_INVALID_TIMESTAMP:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_VIDEO_INVALID_TIMESTAMP\n");
        notifyListener(AML_MP_PLAYER_EVENT_VIDEO_INVALID_TIMESTAMP);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_VIDEO_INVALID_DATA:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_VIDEO_INVALID_DATA\n");
        notifyListener(AML_MP_PLAYER_EVENT_VIDEO_INVALID_DATA);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_AUDIO_INVALID_TIMESTAMP:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_AUDIO_INVALID_TIMESTAMP\n");
        notifyListener(AML_MP_PLAYER_EVENT_AUDIO_INVALID_TIMESTAMP);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_AUDIO_INVALID_DATA:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_AUDIO_INVALID_DATA\n");
        notifyListener(AML_MP_PLAYER_EVENT_AUDIO_INVALID_DATA);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_DECODE_FRAME_ERROR_COUNT:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_DECODE_FRAME_ERROR_COUNT");
        notifyListener(AML_MP_PLAYER_EVENT_VIDEO_ERROR_FRAME_COUNT);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_DECODE_VIDEO_UNSUPPORT:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_DECODE_VIDEO_UNSUPPORT");
        notifyListener(AML_MP_PLAYER_EVENT_VIDEO_UNSUPPORT);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_DECODER_DATA_LOSS:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_DECODER_DATA_LOSS");
        notifyListener(AML_MP_PLAYER_EVENT_DECODER_DATA_LOSS);
    }
    break;

    case AM_TSPLAYER_EVENT_TYPE_DECODER_DATA_RESUME:
    {
        MLOGI("[evt] AM_TSPLAYER_EVENT_TYPE_DECODER_DATA_RESUME");
        notifyListener(AML_MP_PLAYER_EVENT_DECODER_DATA_RESUME);
    }
    break;

    default:
        MLOGE("unhandled event:%d", event->type);
        break;
    }
}

}

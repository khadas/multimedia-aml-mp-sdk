/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef _AML_MP_SW_DEMUX_H_
#define _AML_MP_SW_DEMUX_H_

#include "AmlDemuxBase.h"

namespace aml_mp {
class SwTsParser;
struct AmlMpEventLooper;
struct AmlMpMessage;

template <typename T>
struct AmlMpEventHandlerReflector;

class AmlSwDemux : public AmlDemuxBase
{
public:
    AmlSwDemux();
    ~AmlSwDemux();
    virtual int open(bool isHardwareSource, Aml_MP_DemuxId demuxId, bool isSecureBuffer = false) override;
    virtual int close() override;
    virtual int start() override;
    virtual int stop() override;
    virtual int flush() override;
    virtual int feedTs(const uint8_t* buffer, size_t size) override;

private:
    friend struct AmlMpEventHandlerReflector<AmlSwDemux>;
    enum {
        kWhatFeedData = 'fdta',
        kWhatFlush    = 'flsh',
        kWhatAddPid   = 'apid',
        kWhatRemovePid = 'rpid',
        kWhatDumpInfo = 'dmpI',
    };

    virtual int addDemuxFilter(int pid, const Aml_MP_DemuxFilterParams* params) override;
    virtual int removeDemuxFilter(int pid) override;
    virtual bool isStopped() const override;

    void onMessageReceived(const sptr<AmlMpMessage>& msg);

    void onFeedData(const sptr<AmlMpBuffer>& data);
    int resync(const sptr<AmlMpBuffer>& buffer);
    void onFlush();
    void onAddFilterPid(int pid, Aml_MP_DemuxFilterParams* params);
    void onRemoveFilterPid(int pid);

    sptr<AmlMpEventLooper> mLooper;
    sptr<AmlMpEventHandlerReflector<AmlSwDemux>> mHandler;

    std::atomic<int32_t> mBufferGeneration{0};
    std::atomic<bool> mStopped{false};
    std::atomic<int64_t> mOutBufferCount {0};

    std::mutex mLock;
    sptr<SwTsParser> mTsParser;
    sptr<AmlMpBuffer> mRemainingBytesBuffer;

    int mDumpFd = -1;

private:
    AmlSwDemux(const AmlSwDemux&) = delete;
    AmlSwDemux& operator= (const AmlSwDemux&) = delete;
};
}


#endif

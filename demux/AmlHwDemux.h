/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef _AML_HW_DEMUX_H_
#define _AML_HW_DEMUX_H_

#include "AmlDemuxBase.h"
#include <thread>
#include <map>

namespace android {
class Looper;
}

namespace aml_mp {
using android::sp;
using android::Looper;

class AmlHwDemux : public AmlDemuxBase
{
public:
    AmlHwDemux();
    ~AmlHwDemux();
    int open(bool isHardwareSource, Aml_MP_DemuxId demuxId) override;
    int close() override;
    int start() override;
    int stop() override;
    int flush() override;
    void* createChannel(int pid) override;
    int destroyChannel(void* channel) override;
    int openChannel(void* channel) override;
    int closeChannel(void* channel) override;
    void* createFilter(Aml_MP_Demux_SectionFilterCb cb, void* userData) override;
    int destroyFilter(void* filter) override;
    int attachFilter(void* filter, void* channel) override;
    int detachFilter(void* filter, void* channel) override;

private:
    struct Channel;
    struct Filter;
    struct HwSectionCallback;

    int openHardwareChannel(int pid, int* pFd);
    int closeHardwareChannel(int fd);
    void startPollHardwareChannel(Channel* channel);
    void stopPollHardwareChannel(Channel* channel);

    void threadLoop();

    Aml_MP_DemuxId mDemuxId = AML_MP_DEMUX_ID_DEFAULT;
    std::string mDemuxName;
    std::atomic<bool> mStopped;

    std::atomic<uint32_t> mFilterId;
    sp<HwSectionCallback> mHwSectionCallback;

    std::thread mThread;
    sp<Looper> mLooper;

    std::mutex mLock;
    std::map<int, sp<Channel>> mChannels;

    AmlHwDemux(const AmlHwDemux&) = delete;
    AmlHwDemux& operator= (const AmlHwDemux&) = delete;
};

}


#endif

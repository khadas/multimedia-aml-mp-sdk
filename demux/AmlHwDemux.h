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
#include <set>
#include <utils/AmlMpLooper.h>

namespace aml_mp {
class HwTsParser;

class AmlHwDemux : public AmlDemuxBase
{
public:
    struct FilterParams {
        int pid;
        int fd;
        Aml_MP_DemuxFilterParams params;
    };

    AmlHwDemux();
    ~AmlHwDemux();
    int open(bool isHardwareSource, Aml_MP_DemuxId demuxId, bool isSecureBuffer = false) override;
    int close() override;
    int start() override;
    int stop() override;
    int flush() override;
    virtual int feedTs(const uint8_t* buffer, size_t size) override;

private:
    void threadLoop();
    int addDemuxFilter(int pid, const Aml_MP_DemuxFilterParams* params) override;
    int removeDemuxFilter(int pid) override;
    bool isStopped() const override;

    Aml_MP_DemuxId mDemuxId = AML_MP_DEMUX_ID_DEFAULT;
    std::string mDemuxName;
    std::thread mThread;
    sptr<Looper> mLooper;
    sptr<HwTsParser> mTsParser;
    bool mIsHardwareSource;
    bool mIsSecureBuffer;

    std::atomic<bool> mStopped{};

    std::map<int, std::unique_ptr<FilterParams>> mFilterParams;

private:
    AmlHwDemux(const AmlHwDemux&) = delete;
    AmlHwDemux& operator= (const AmlHwDemux&) = delete;
};

}


#endif

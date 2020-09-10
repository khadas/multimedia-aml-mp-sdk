#ifndef _UDP_SOURCE_H_
#define _UDP_SOURCE_H_

#include "Source.h"
#include <string>
#include <thread>
#include <utils/AmlMpFifo.h>
#include <mutex>
#include <condition_variable>

struct addrinfo;

namespace android {
class Looper;
}

namespace aml_mp {
using android::sp;
using android::Looper;

class UdpSource : public Source
{
public:
    UdpSource(const char* address, int programNumber, uint32_t flags);
    ~UdpSource();

    virtual int initCheck() override;
    virtual int start() override;
    virtual int stop() override;
    virtual void signalQuit() override;

private:
    void readThreadLoop();
    void feedThreadLoop();
    void doStatistic(int size);

    std::string mAddress;
    struct addrinfo* mAddrInfo = nullptr;
    int mSocket = -1;
    std::thread mReadThread;
    sp<Looper> mLooper;

    std::thread mFeedThread;
    std::mutex mFeedLock;
    std::condition_variable mFeedCond;
    AmlMpFifo mFifo;
    uint32_t mFeedWork{};

    enum FeedWork {
        kWorkFeedData   = 1 << 0,
        kWorkQuit       = 1 << 1,
    };

    void sendFeedWorkCommand(FeedWork work) {
        std::lock_guard<std::mutex> _l(mFeedLock);
        mFeedWork |= work;
        mFeedCond.notify_all();
    }

    void signalFeedWorkDone(FeedWork work) {
        std::lock_guard<std::mutex> _l(mFeedLock);
        mFeedWork &= ~ work;
    }

    static const int SOCKET_FD_IDENTIFIER = 0x100;

    int mDumpFd = -1;

    std::atomic_bool mRequestQuit = false;

    int64_t mLastBitRateMeasureTime = -1;
    int64_t mBitRateMeasureSize = 0;

private:
    UdpSource(const UdpSource&) = delete;
    UdpSource& operator= (const UdpSource&) = delete;
};



}
#endif

/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_TAG "AmlMpPlayerVideoTest"
#include "AmlMpPlayerVideoTest.h"
#include <utils/AmlMpLog.h>
#include <utils/AmlMpUtils.h>
#include <gtest/gtest.h>
#include "TestUrlList.h"
#include "../amlMpTestSupporter/TestModule.h"
#include <AmlMpTestSupporter.h>
#include <pthread.h>
#include <getopt.h>
#include "source/Source.h"
#include <Aml_MP/Aml_MP.h>
#include "string.h"
#include <unistd.h>


using namespace aml_mp;

int amsysfs_get_sysfs_str(const char *path, char* buf, size_t len) {
    int fd;
    int bytes;
    fd = open(path, O_RDONLY);
    if (fd >= 0)
    {
        bytes = read(fd, buf, len);
        close(fd);
        return 0;
    }
    return -1;
}

std::string get_vfm_map_checkpoint()
{
    char buf[1024];
    if (amsysfs_get_sysfs_str("/sys/class/vfm/map", buf, sizeof(buf)) < 0)
    {
        return "";
    }
    return std::string(buf);
}

std::string get_frame_count_checkpoint()
{
    char buf[1024];
    if (amsysfs_get_sysfs_str("/sys/class/video_composer/receive_count", buf, sizeof(buf)) < 0)
    {
        return "";
    }
    return std::string(buf);
}

void check_frame_count(int timeoutMs)
{
    std::string frame = "1";
    std::string frame_temp = "1";
    int i = AML_MP_OK;
    while (i < kCheckFrameTimeOutMs)
    {
        sleep(kSleepTimeMs);
        frame = get_frame_count_checkpoint();
        MLOGI("frame is %s", frame.c_str());
        MLOGI("frame_temp is %s", frame_temp.c_str());
        if (frame > frame_temp)
        {
            EXPECT_TRUE(frame > frame_temp);
            frame_temp = frame;
        }else{
            EXPECT_FALSE(frame > frame_temp);
        }
        i++;
    }
}

void check_vfm_map(int timeoutMs)
{
    std::string ret;
    clock_t t;
    t = clock();
    while (clock() - t < timeoutMs)
    {
        ret = get_vfm_map_checkpoint();
        EXPECT_TRUE(ret.find(VCOM_MAP) != std::string::npos);
        std::smatch m;
        std::regex vdec_map_pattern ("^.* \\[\\d+\\]\\s+vdec-map-1 \\{\\s+vdec\\.\\w+\\.\\d+\\(1\\) dimulti\\.1\\(1\\) v4lvideo\\.0\\} .*$");
        while (std::regex_search (ret, m, vdec_map_pattern))
        {
            for (auto x:m)
            {
                std::string ret;
                std::cout << x << std::endl;
                EXPECT_TRUE(ret.find(x) != std::string::npos);
                return;
            }
        }
    }
}

void AmlMpPlayerBase::createMpTestSupporter()
{
    if (mpTestSupporter == nullptr)
    {
        mpTestSupporter     = new AmlMpTestSupporter;
        mpTestSupporter->registerEventCallback([] (void * userData, Aml_MP_PlayerEventType event, int64_t param){ AmlMpPlayerBase * self = (AmlMpPlayerBase *) userData; return self->eventCallback(event, param); }, this);
    }
}

void AmlMpPlayerBase::checkHasVideo(std::string url)
{
    if (mpTestSupporter->hasVideo())
    {
        EXPECT_TRUE(waitFirstVFrameEvent()) << defaultFailureMessage(url);
        EXPECT_TRUE(waitVideoChangedEvent(kWaitVideoChangedMs));
    }
}

template<typename T1, typename T2>
void AmlMpPlayerBase::ParameterTest(const std::string & url, T1 key, T2 parameter)
{
    MLOGI("ParameterTest START: %d \n", parameter);
    runTest(url, false);
    if (mRet == 0)
    {
        void *player = getPlayer();
        EXPECT_EQ(Aml_MP_Player_SetParameter(player, key, &parameter), Aml_MP_Player_GetParameter(player, key, &parameter));
        EXPECT_FALSE(waitPlayingErrors());
    }
    MLOGI("----------ParameterTest END----------\n");
    stopPlaying();
}

void AmlMpPlayerBase::SetVideoWindow(const std::string & url, int32_t x, int32_t y, int32_t width, int32_t height)
{
    runTest(url, false);
    if (mRet == 0)
    {
        void *player = getPlayer();
        EXPECT_EQ(Aml_MP_Player_SetVideoWindow(player, x, y, width, height), AML_MP_OK);
        EXPECT_FALSE(waitPlayingErrors());
    }
    MLOGI("----------ParameterTest END----------\n");
    stopPlaying();
}

void AmlMpPlayerBase::runTest(const std::string & url, bool isStop)
{
    startPlaying(url);
    checkHasVideo(url);
    EXPECT_FALSE(waitPlayingErrors());
    if (isStop) {
        stopPlaying();
    }
}

void AmlMpPlayerBase::startPlaying(const std::string & url)
{
    MLOGI("url:%s", url.c_str());
    mRet = 0;
    createMpTestSupporter();
    mpTestSupporter->setDataSource(url);
    mRet = mpTestSupporter->prepare();
    if (mRet != 0)
    {
        return;
    }
    //EXPECT_EQ(ret, 0) << defaultFailureMessage(url);
    int ret = mpTestSupporter->startPlay();
    EXPECT_EQ(ret, 0) << defaultFailureMessage(url);
    if (ret != 0)
    {
        mPlayingHaveErrors = true;
    }
}

void AmlMpPlayerBase::stopPlaying()
{
    mpTestSupporter->stop();
    mFirstVFrameDisplayed = false;
    mPlayingHaveErrors = false;
}

bool AmlMpPlayerBase::waitFirstVFrameEvent(int timeoutMs)
{
    std::unique_lock <std::mutex> _l(mLock);
    return mCond.wait_for(_l, std::chrono::milliseconds(timeoutMs), [this] () { return mFirstVFrameDisplayed; });
}

bool AmlMpPlayerBase::waitVideoChangedEvent(int timeoutMs)
{
    std::unique_lock <std::mutex> _l(mLock);
    return mCond.wait_for(_l, std::chrono::milliseconds(timeoutMs), [this] () { return mVideoChanged; });
}

bool AmlMpPlayerBase::waitDataLossEvent(int timeoutMs)
{
    std::unique_lock <std::mutex> _l(mLock);
    return mCond.wait_for(_l, std::chrono::milliseconds(timeoutMs), [this] () { return mDataLoss; });
}

bool AmlMpPlayerBase::waitPlayingErrors(int timeoutMs)
{
    std::unique_lock <std::mutex> _l(mLock);
    std::thread checkvfmmap (check_vfm_map, timeoutMs);
    std::thread checkframecount (check_frame_count, timeoutMs);
    checkvfmmap.detach();
    checkframecount.detach();
    return mCond.wait_for(_l, std::chrono::milliseconds(timeoutMs), [this] () { return mPlayingHaveErrors; });
}

bool AmlMpPlayerBase::waitPlaying(int timeoutMs)
{
    std::unique_lock <std::mutex> _l(mLock);
    return mCond.wait_for(_l, std::chrono::milliseconds(timeoutMs), [this] () { return mPlayingHaveErrors; });
}

void AmlMpPlayerBase::eventCallback(Aml_MP_PlayerEventType event, int64_t param)
{
std::unique_lock <std::mutex> _l(mLock);
    switch (event)
    {
        case AML_MP_PLAYER_EVENT_FIRST_FRAME:
        {
            mFirstVFrameDisplayed = true;
            mCond.notify_all();
            break;
        }
        case AML_MP_PLAYER_EVENT_VIDEO_CHANGED:
        {
            mVideoChanged = true;
            MLOGI("video changed-----");
            mCond.notify_all();
            break;
        }
        case AML_MP_PLAYER_EVENT_AUDIO_CHANGED:
        {
            mAudioChanged = true;
            MLOGI("audio changed-----");
            mCond.notify_all();
            break;
        }
        case AML_MP_PLAYER_EVENT_DATA_LOSS:
        {
            mDataLoss = true;
            MLOGI("data loss-----");
            mCond.notify_all();
            break;
        }
        default:
            break;
    }
}

AML_MP_PLAYER AmlMpPlayerBase::getPlayer()
{
    sptr <TestModule> testModule = mpTestSupporter->getPlayback();
    AML_MP_PLAYER player = testModule->getCommandHandle();
    return player;
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(AmlMpPlayerTest, startPlay_StartAll_StopAll_Test)
{
    std::string url;
    for (auto &url: mUrls)
    {
        mPlayMode = AmlMpTestSupporter::START_ALL_STOP_ALL;
        runTest(url);
    }
}

TEST_F(AmlMpPlayerTest, startPlay_StartAll_StopSeparately_Test)
{
    std::string url;
    for (auto &url: mUrls)
    {
        mPlayMode = AmlMpTestSupporter::START_ALL_STOP_SEPARATELY;
        runTest(url);
    }
}

TEST_F(AmlMpPlayerTest, startPlay_startSeparately_StopAll_Test)
{
    std::string url;
    for (auto &url: mUrls)
    {
        mPlayMode = AmlMpTestSupporter::START_SEPARATELY_STOP_ALL;
        runTest(url);
    }
}

TEST_F(AmlMpPlayerTest, startPlay_StartSeparately_StopSeparately_Test)
{
    std::string url;
    for (auto &url: mUrls)
    {
        mPlayMode = AmlMpTestSupporter::START_SEPARATELY_STOP_SEPARATELY;
        runTest(url);
    }
}

TEST_F(AmlMpPlayerTest, startPlay_StartSeparately_StopSeparately_V2_Test)
{
    std::string url;
    for (auto &url: mUrls)
    {
        mPlayMode = AmlMpTestSupporter::START_SEPARATELY_STOP_SEPARATELY_V2;
        runTest(url);
    }
}

TEST_F(AmlMpPlayerTest, startPlay_StartAudio_Video_Test)
{
    std::string url;
    for (auto &url: mUrls)
    {
        mPlayMode = AmlMpTestSupporter::START_AUDIO_START_VIDEO;
        runTest(url);
    }
}

TEST_F(AmlMpPlayerTest, startPlay_StartVideo_AudioTest)
{
    std::string url;
    for (auto &url: mUrls)
    {
        mPlayMode = AmlMpTestSupporter::START_VIDEO_START_AUDIO;
        runTest(url);
    }
}

TEST_F(AmlMpPlayerTest, startPlay_ModeMax_Test)
{
    std::string url;
    for (auto &url: mUrls)
    {
        mPlayMode = AmlMpTestSupporter::PLAY_MODE_MAX;
        runTest(url);
    }
}

TEST_F(AmlMpPlayerTest, clearLastFrame)
{
    std::string url;
    for (auto &url: mUrls)
    {
        startPlaying(url);
        checkHasVideo(url);
        if (mRet != 0)
        {
            continue;
        }
        EXPECT_FALSE(waitPlayingErrors());
        void *player = getPlayer();
        int blackOut = 1;
        Aml_MP_Player_SetParameter(player, AML_MP_PLAYER_PARAMETER_BLACK_OUT, &blackOut);
        EXPECT_FALSE(waitPlayingErrors());
        stopPlaying();
    }
}

TEST_F(AmlMpPlayerTest, ZappingTest)
{
    std::string url;
    for (auto &url: mUrls)
    {
        runTest(url);
    }
}

TEST_F(AmlMpPlayerTest, showHideVideoTest)
{
    std::string url;
    for (auto &url: mUrls)
    {
        startPlaying(url);
        if (mRet == 0)
        {
            checkHasVideo(url);
            EXPECT_FALSE(waitPlayingErrors());
            void *player = getPlayer();
            EXPECT_EQ(Aml_MP_Player_HideVideo(player), AML_MP_OK);
            EXPECT_FALSE(waitPlayingErrors());
            EXPECT_EQ(Aml_MP_Player_ShowVideo(player), AML_MP_OK);
            EXPECT_FALSE(waitPlayingErrors());
        }
        stopPlaying();
    }
}

TEST_F(AmlMpPlayerTest, setANativeWindowTest)
{
    std::string url;
    for (auto &url: mUrls)
    {
        MLOGI("----------SetANativeWindow START----------");
        startPlaying(url);
        if (mRet == 0)
        {
            checkHasVideo(url);
            EXPECT_FALSE(waitPlayingErrors());
            void *player = getPlayer();
            sptr <NativeUI> mNativeUI = mpTestSupporter->createNativeUI();
            sp <ANativeWindow> mSurface = mNativeUI->getNativeWindow();
            EXPECT_EQ(Aml_MP_Player_SetANativeWindow(player, mSurface.get()), AML_MP_OK);
            MLOGI("----------SetANativeWindow END----------");
        }
        stopPlaying();
    }
}

TEST_F(AmlMpPlayerTest, surfaceControlTest)
{
    std::string url;
    createMpTestSupporter();
    #define AML_MP_VIDEO_MODE 0
    //set surface
    AmlMpTestSupporter::DisplayParam displayParam;
    displayParam.x      = AML_MP_VIDEO_WINDOW_X;
    displayParam.y      = AML_MP_VIDEO_WINDOW_Y;
    displayParam.width  = AML_MP_VIDEO_WINDOW_WIDTH;
    displayParam.height = AML_MP_VIDEO_WINDOW_HEIGHT;
    displayParam.zorder = AML_MP_VIDEO_WINDOW_ZORDER;
    displayParam.videoMode = AML_MP_VIDEO_MODE;
    mpTestSupporter->setDisplayParam(displayParam);
    for (auto &url: mUrls)
    {
        MLOGI("----------surfaceControlTest START----------");
        runTest(url);
        MLOGI("----------surfaceControlTest END----------");
    }
}

TEST_F(AmlMpPlayerTest, setVideoMode_1_Test)
{
    std::string url;
    createMpTestSupporter();
    AmlMpTestSupporter::DisplayParam displayParam;
    displayParam.x = AML_MP_VIDEO_WINDOW_X;
    displayParam.y = AML_MP_VIDEO_WINDOW_Y;
    displayParam.width = AML_MP_VIDEO_WINDOW_WIDTH;
    displayParam.height = AML_MP_VIDEO_WINDOW_HEIGHT;
    displayParam.zorder = AML_MP_VIDEO_WINDOW_ZORDER;
    displayParam.videoMode = AML_MP_VIDEO_MODE;
    mpTestSupporter->setDisplayParam(displayParam);
    for (auto &url: mUrls)
    {
        MLOGI("----------setVideoMode_1_Test START----------");
        SetVideoWindow(url, displayParam.x, displayParam.y, displayParam.width, displayParam.height);
        MLOGI("----------setVideoMode_1_Test END----------");
    }
}

TEST_F(AmlMpPlayerTest, setVideoMode_2_Test)
{
    std::string url;
    createMpTestSupporter();
    #define AML_MP_VIDEO_WINDOW_ZORDER 2
    AmlMpTestSupporter::DisplayParam displayParam;
    displayParam.x      = AML_MP_VIDEO_WINDOW_X;
    displayParam.y      = AML_MP_VIDEO_WINDOW_Y;
    displayParam.width  = AML_MP_VIDEO_WINDOW_WIDTH;
    displayParam.height = AML_MP_VIDEO_WINDOW_HEIGHT;
    displayParam.zorder = AML_MP_VIDEO_WINDOW_ZORDER;
    displayParam.videoMode = AML_MP_VIDEO_MODE;
    mpTestSupporter->setDisplayParam(displayParam);
    for (auto &url: mUrls)
    {
        MLOGI("----------setVideoMode_2_Test START----------");
        SetVideoWindow(url, displayParam.x, displayParam.y, displayParam.width, displayParam.height);
        MLOGI("----------setVideoMode_2_Test END----------");
    }
}

TEST_F(AmlMpPlayerTest, videoTunnelID_Test)
{
    std::string url;
    createMpTestSupporter();
    //set Surface
    AmlMpTestSupporter::DisplayParam displayParam;
    displayParam.x      = AML_MP_VIDEO_WINDOW_X;
    displayParam.y      = AML_MP_VIDEO_WINDOW_Y;
    displayParam.width  = AML_MP_VIDEO_WINDOW_WIDTH;
    displayParam.height = AML_MP_VIDEO_WINDOW_HEIGHT;
    mpTestSupporter->setDisplayParam(displayParam);
    for (auto &url: mUrls)
    {
        MLOGI("----------videoTunnelIDTest START----------");
        startPlaying(url);
        checkHasVideo(url);
        if (mRet != 0)
        {
            continue;
        }
        EXPECT_FALSE(waitPlayingErrors());
        void *player = getPlayer();
        EXPECT_EQ(Aml_MP_Player_SetParameter(player, AML_MP_PLAYER_PARAMETER_VIDEO_TUNNEL_ID, &displayParam),Aml_MP_Player_GetParameter(player, AML_MP_PLAYER_PARAMETER_VIDEO_TUNNEL_ID, &displayParam));
        EXPECT_FALSE(waitPlayingErrors());
        MLOGI("----------videoTunnelIDTest END----------");
        stopPlaying();
    }
}

TEST_F(AmlMpPlayerTest, displayModeTest)
{
    std::string url;
    for (auto &url: mUrls)
    {
        for (Aml_MP_VideoDisplayMode displayModeEnum = AML_MP_VIDEO_DISPLAY_MODE_NORMAL; displayModeEnum <= AML_MP_VIDEO_DISPLAY_MODE_CUSTOM;
            displayModeEnum = (Aml_MP_VideoDisplayMode) (displayModeEnum + 1))
            {
                MLOGI("displayModeEnum is %d \n", displayModeEnum);
                ParameterTest(url, AML_MP_PLAYER_PARAMETER_VIDEO_DISPLAY_MODE, displayModeEnum);
            }
    }
}

TEST_F(AmlMpPlayerTest, videoDecodeModeTest)
{
    std::string url;
    for (auto &url: mUrls)
    {
        for (Aml_MP_VideoDecodeMode videodecodeModeEnum = AML_MP_VIDEO_DECODE_MODE_NONE; videodecodeModeEnum <= AML_MP_VIDEO_DECODE_MODE_IONLY;
            videodecodeModeEnum = (Aml_MP_VideoDecodeMode) (videodecodeModeEnum + 1))
            {
                MLOGI("videodecodeModeEnum is %d \n", videodecodeModeEnum);
                ParameterTest(url, AML_MP_PLAYER_PARAMETER_VIDEO_DECODE_MODE, videodecodeModeEnum);
            }
    }
}

///////////////////////////////////////////////////////////////////////////////
static void usage()
{
    printf("Usage: AmlMpPlayerTest <test_source_dir>\n"
    "   try --help for more details.\n");
}

int main(int argc, char * argv[])
{
    if (argc == 1)
    {
        usage();
        return 0;
    }
    std::string testpath;
    for (size_t i = 1; i < argc;)
    {
        if (argv[i][0] != '-')
        {
            testpath = argv[i];
            for (int j = i; j < argc - 1; ++j)
            {
                argv[j] = argv[j + 1];
            }
            argc--;
        }
        else
        {
            ++i;
        }
    }
    testing::InitGoogleTest(&argc, argv);
    if (!testpath.empty())
    {
        printf("testpath:%s, argc:%d\n", testpath.c_str(), argc);
        TestUrlList::instance().initSourceDir(testpath);
    }
    return RUN_ALL_TESTS();
}

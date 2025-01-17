/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#define LOG_TAG "AmlMpPlayerDemo_TestUtils"
#include <utils/AmlMpLog.h>
#include <utils/AmlMpUtils.h>
#include "TestUtils.h"
#include <signal.h>
#include <thread>
#include <poll.h>
#include <unistd.h>

static const char* mName = LOG_TAG;

namespace aml_mp {
NativeUI::NativeUI()
{
#ifdef ANDROID
#ifndef __ANDROID_VNDK__
    mComposerClient = new android::SurfaceComposerClient;
    AML_MP_CHECK_EQ(mComposerClient->initCheck(), android::OK);

    //sp<android::IBinder> displayToken  = nullptr;
    //displayToken = mComposerClient->getInternalDisplayToken();

    //android::DisplayInfo displayInfo;
    //CHECK_EQ(OK, mComposerClient->getDisplayInfo(displayToken, &displayInfo));
    //mDisplayWidth = displayInfo.w;
    //mDisplayHeight = displayInfo.h;
    mDisplayWidth = mSurfaceWidth;
    mDisplayHeight = mSurfaceHeight;
    MLOGI("mDisplayWidth: %d, mDisplayHeight: %d", mDisplayWidth, mDisplayHeight);
    //mSurfaceWidth = mDisplayWidth >> 1;
    //mSurfaceHeight = mDisplayHeight >> 1;
    //MLOGI("mSurfaceWidth: %d, mSurfaceHeight: %d", mSurfaceWidth, mSurfaceHeight);


    mSurfaceControl = mComposerClient->createSurface(android::String8("AmlMpPlayer"), mSurfaceWidth, mSurfaceHeight, android::PIXEL_FORMAT_RGB_565, 0);
    AML_MP_CHECK(mSurfaceControl->isValid());
    mSurface = mSurfaceControl->getSurface();

    android::SurfaceComposerClient::Transaction()
        .setFlags(mSurfaceControl, android::layer_state_t::eLayerOpaque, android::layer_state_t::eLayerOpaque)
#if ANDROID_PLATFORM_SDK_VERSION >= 31
        .setCrop(mSurfaceControl, android::Rect(mSurfaceWidth, mSurfaceHeight))
#elif ANDROID_PLATFORM_SDK_VERSION >= 29
        .setCrop_legacy(mSurfaceControl, android::Rect(mSurfaceWidth, mSurfaceHeight))
#endif
        .show(mSurfaceControl)
        .apply();

    mSurfaceControlUi = mComposerClient->createSurface(android::String8("AmlMpPlayer-ui"), mDisplayWidth, mDisplayHeight, android::PIXEL_FORMAT_RGBA_8888, 0);
    AML_MP_CHECK(mSurfaceControlUi->isValid());
    mSurfaceUi = mSurfaceControlUi->getSurface();

    int ret = native_window_api_connect(mSurfaceUi.get(), NATIVE_WINDOW_API_CPU);
    if (ret < 0) {
        MLOGE("mSurfaceUi connect failed with %d!", ret);
        return;
    }

    mSurfaceUi->allocateBuffers();

    android::SurfaceComposerClient::Transaction()
#if ANDROID_PLATFORM_SDK_VERSION >= 31
        .setCrop(mSurfaceControlUi, android::Rect(mDisplayWidth, mDisplayHeight))
#elif ANDROID_PLATFORM_SDK_VERSION >= 29
        .setCrop_legacy(mSurfaceControlUi, android::Rect(mDisplayWidth, mDisplayHeight))
#endif
        .setLayer(mSurfaceControlUi, UI_LAYER)
        .show(mSurfaceControlUi)
        .apply();

    ANativeWindow* nativeWindow = static_cast<ANativeWindow*>(mSurfaceUi.get());

    int err = 0;
    ANativeWindowBuffer* buf;
    err = nativeWindow->dequeueBuffer_DEPRECATED(nativeWindow, &buf);
    if (err != 0) {
        MLOGE("dequeueBuffer failed:%d\n", err);
        return;
    }

    nativeWindow->lockBuffer_DEPRECATED(nativeWindow, buf);
    sp<android::GraphicBuffer> graphicBuffer = android::GraphicBuffer::from(buf);

    char* vaddr;
    graphicBuffer->lock(1, (void **)&vaddr);
    if (vaddr != nullptr) {
        memset(vaddr, 0x0, graphicBuffer->getWidth() * graphicBuffer->getHeight() * 4); /*to show video in osd hole...*/
    }
    graphicBuffer->unlock();
    graphicBuffer.clear();
    nativeWindow->queueBuffer_DEPRECATED(nativeWindow, buf);
#endif
#else
    mSurfaceWidth = 0;
    mSurfaceHeight = 0;
#endif
}

NativeUI::~NativeUI()
{

}

#ifdef ANDROID
sp<ANativeWindow> NativeUI::getNativeWindow() const
{
#ifndef __ANDROID_VNDK__
    return mSurface;
#else
    return nullptr;
#endif
}
#endif

void NativeUI::controlSurface(int zorder)
{
#ifdef ANDROID
#ifndef __ANDROID_VNDK__
    auto transcation = android::SurfaceComposerClient::Transaction();

    transcation.setLayer(mSurfaceControl, zorder);
    transcation.setLayer(mSurfaceControlUi, zorder);

    transcation.apply();
#else
    AML_MP_UNUSED(zorder);
#endif
#endif
}

void NativeUI::controlSurface(int left, int top, int right, int bottom)
{
#ifdef ANDROID
#ifndef __ANDROID_VNDK__
    auto transcation = android::SurfaceComposerClient::Transaction();

    if (left >= 0 && top >= 0) {
        transcation.setPosition(mSurfaceControl, left, top);
    }

    if (right > left && bottom > top) {
        int width = right - left;
        int height = bottom - top;
        transcation.setSize(mSurfaceControl, width, height);
#if ANDROID_PLATFORM_SDK_VERSION >= 31
        transcation.setCrop(mSurfaceControl, android::Rect(width, height));
#elif ANDROID_PLATFORM_SDK_VERSION >= 29
        transcation.setCrop_legacy(mSurfaceControl, android::Rect(width, height));
#endif
    }

    transcation.apply();
#else
    AML_MP_UNUSED(left);
    AML_MP_UNUSED(top);
    AML_MP_UNUSED(right);
    AML_MP_UNUSED(bottom);
#endif
#endif
}

int NativeUI::getDefaultSurfaceWidth() const
{
    return mSurfaceWidth;
}

int NativeUI::getDefaultSurfaceHeight() const
{
    return mSurfaceHeight;
}

CommandProcessor::CommandProcessor(const std::string& prompt)
: mPrompt(prompt)
{

}

CommandProcessor::~CommandProcessor()
{

}

int CommandProcessor::setCommandVisitor(const std::function<Visitor>& visitor)
{
    mCommandVisitor = visitor;

    return 0;
}

int CommandProcessor::setInterrupter(const std::function<Interrupter>& interrupter)
{
    mInterrupter = interrupter;

    return 0;
}

int CommandProcessor::fetchAndProcessCommands()
{
    bool prompt = false;
    std::string lastCommand;

    for (;;) {
        if (mInterrupter()) {
            break;
        }

        if (tcgetpgrp(0) != getpid()) {
            usleep(10 * 1000);
            continue;
        }

        if (prompt) {
            prompt = false;
            fprintf(stderr, "%s", mPrompt.c_str());
        }

        struct pollfd fds;
        fds.fd = STDIN_FILENO;
        fds.events = POLL_IN;
        fds.revents = 0;
        // change the wait time from 1s to 100ms to avoid
        // the situation that the signal change is not detected in time
        int ret = ::poll(&fds, 1, 100);
        if (ret < 0) {
            //printf("poll STDIN_FILENO failed! %d\n", -errno);
        } else if (ret > 0) {
            if (fds.revents & POLL_ERR) {
                MLOGE("poll error!");
                continue;
            } else if (!(fds.revents & POLL_IN)) {
                continue;
            }

            prompt = true;

            char buffer[100]{0};
            int len = ::read(STDIN_FILENO, buffer, sizeof(buffer));
            if (len <= 0) {
                MLOGE("read failed! %d", -errno);
                continue;
            }
            buffer[len-1] = '\0';
            //printf("read buf:%s, size:%d\n", buffer, len);

            std::string buf(buffer);
            size_t b = 0;
            size_t e = buf.size();
            while (b < e) {
                if (isspace(buf[b])) ++b;
                else if (isspace(buf[e])) --e;
                else break;
            }

            if (b < e) {
                buf = buf.substr(b, e - b);
                lastCommand = buf;
            } else if (b == e && !lastCommand.empty()) {
                buf = lastCommand;
            } else {
                continue;
            }

            std::vector<std::string> args = split(buf);
            mCommandVisitor(args);
        }
    }

    return 0;
}

std::vector<std::string> CommandProcessor::split(const std::string& str)
{
    std::vector<std::string> result;
    std::string::size_type pos, prev = 0;
    std::string s;
    std::string cmdDelimiter = " ";
    std::string argDelimiter = ", ";

    pos = str.find_first_of(cmdDelimiter);

    if (pos == std::string::npos) {
        result.emplace_back(std::move(str));
        return result;
    }

    result.emplace_back(str, 0, pos);
    prev = pos + 1;

    while ((pos = str.find_first_of(argDelimiter, prev)) != std::string::npos) {
        if (pos > prev) {
            size_t b = prev;
            size_t e = pos;
            while (b < e) {
                if (isspace(str[b]) || isDelimiter(argDelimiter, str[b]))        ++b;
                else if (isspace(str[e]) || isDelimiter(argDelimiter, str[e]))   --e;
                else break;
            }

            if (b <= e) {
                result.emplace_back(str, b, e - b + 1);
            }
        }

        prev = pos + 1;
    }

    while (prev < str.size()) {
        if (isspace(str[prev]) || isDelimiter(argDelimiter, str[prev]))
            ++prev;
        else
            break;
    }

    if (prev < str.size())
        result.emplace_back(str, prev, str.size() - prev);

    return result;
}


}

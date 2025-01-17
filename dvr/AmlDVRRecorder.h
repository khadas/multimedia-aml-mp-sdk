/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef _AML_DVR_RECODER_H_
#define _AML_DVR_RECODER_H_

#include <Aml_MP/Dvr.h>
#include <utils/AmlMpHandle.h>
#include <utils/AmlMpUtils.h>

namespace aml_mp {

class AmlDVRRecorder final : public AmlMpHandle
{
public:
    AmlDVRRecorder(Aml_MP_DVRRecorderBasicParams* basicParams, Aml_MP_DVRRecorderTimeShiftParams* timeShiftParams = nullptr, Aml_MP_DVRRecorderEncryptParams* encryptParams = nullptr);
    ~AmlDVRRecorder();
    int registerEventCallback(Aml_MP_DVRRecorderEventCallback cb, void* userData);
    int setStreams(Aml_MP_DVRStreamArray* streams);
    int start();
    int stop();
    int pause();
    int resume();
    int getStatus(Aml_MP_DVRRecorderStatus* status);
    int isSecureMode() const;
    int setEncryptParams(Aml_MP_DVRRecorderEncryptParams* encryptParams);

private:
    int setBasicParams(Aml_MP_DVRRecorderBasicParams* basicParams);
    int setTimeShiftParams(Aml_MP_DVRRecorderTimeShiftParams* timeShiftParams);
    int setSharedParams(Aml_MP_DVRRecorderBasicParams* basicParams);

    DVR_Result_t eventHandler(DVR_RecordEvent_t event, void* params);

    char mName[50];
    DVR_WrapperRecordOpenParams_t mRecOpenParams{};
    DVR_WrapperRecordStartParams_t mRecStartParams{};
    DVR_WrapperRecord_t mRecoderHandle = nullptr;
    bool mStarted = false;
    uint8_t* mSecureBuffer = nullptr;
    size_t mSecureBufferSize = 0;

    DVR_WrapperPidsInfo_t mRecordPids;

    Aml_MP_DVRRecorderEventCallback mEventCb = nullptr;
    void* mEventUserData = nullptr;
    Aml_MP_CB_Data mSharedCb = nullptr;
    void* mSharedUserData = nullptr;
    bool mIsOutData;

private:
    AmlDVRRecorder(const AmlDVRRecorder&) = delete;
    AmlDVRRecorder& operator= (const AmlDVRRecorder&) = delete;
};

}
#endif

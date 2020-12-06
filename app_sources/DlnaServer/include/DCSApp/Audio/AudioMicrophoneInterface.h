/*
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef DUEROS_DCS_APP_SAMPLEAPP_AUDIOMICROPHONEINTERFACE_H_
#define DUEROS_DCS_APP_SAMPLEAPP_AUDIOMICROPHONEINTERFACE_H_
#include <mutex>
#include <thread>
#include "fstream"
#include "LoggerUtils/DcsSdkLogger.h"
#include <libcchip/platform.h>
#include <vepinput/vepinput.h>

namespace duerOSDcsApp {
namespace application {
#define BACKUP_THE_UPLOADED_DATE_PATH "/tmp/snd.raw"

/// the Audio Microphone interface.
class AudioMicrophoneInterface {
public:
    /**
     * Stops streaming from the microphone.
     *
     * @return Whether the stop was successful.
     */
    virtual bool stopStreamingMicrophoneData() = 0;

    /**
     * Starts streaming from the microphone.
     *
     * @return Whether the start was successful.
     */
    virtual bool startStreamingMicrophoneData() = 0;

    virtual void setRecordDataInputCallback(void (*callBack)(const char *buffer, unsigned int size)) = 0;
	virtual void excuteExpectSpeechResponse(void) = 0;
	virtual void excuteWakeupResponse(void) = 0;
	virtual void cmccVoipRcordingstarted(void) = 0;
	virtual void cmccVoipRcordingStoped(void)  = 0;
	virtual void checkAndStopMutiVad(void) = 0;
	std::ofstream backupFile;
	std::ofstream wakeupTimes;
	size_t wakeupTimesCount=0;
	vad_args_t vad_args;
	volatile bool isCmccVoipRcording;
	volatile bool isCloudVadListening;
	volatile bool isLocalVadListening;
	volatile bool isSoundResponseDone;
	volatile size_t throwInvaildCount;
	volatile size_t throwInvaildMax;
	vepinput_t* _vepinput;
	csrb_t * m_csrb;
	shrb_t * _shrb;
	trd_timer_t *_trdTimer;
};
}
}
#endif //DUEROS_DCS_APP_SAMPLEAPP_AUDIOMICROPHONEINTERFACE_H_

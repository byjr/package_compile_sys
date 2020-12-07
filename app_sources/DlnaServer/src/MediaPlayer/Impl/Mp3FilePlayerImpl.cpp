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

#include <unistd.h>
#include "Mp3FilePlayerImpl.h"
#include "DeviceTools/Utils.h"
#include "LoggerUtils/DcsSdkLogger.h"

namespace duerOSDcsApp {
namespace mediaPlayer {

#define MAX_AUDIO_FRAME_SIZE 22050
#define OUTPUT_CHANNEL 1
#define OUTPUT_SAMPLE_RATE 44100
#define FRAME_BUFFER_SIZE 44100

std::shared_ptr<Mp3FilePlayerImpl> Mp3FilePlayerImpl::create(const std::string& alertsAudioDev, const std::string& promptAudioDev) {
    std::shared_ptr<Mp3FilePlayerImpl> instance(new Mp3FilePlayerImpl(alertsAudioDev, promptAudioDev));
    return instance;
}

Mp3FilePlayerImpl::Mp3FilePlayerImpl(const std::string& alertsAudioDev, const std::string& promptAudioDev) : m_executor(NULL),
    m_alertsAlsa(NULL),
    m_promptAlsa(NULL),
    m_audioDecoder(NULL),
    m_alertsListener(NULL),
    m_promptListener(NULL),
    m_currentListener(NULL),
    m_pcmBuffer(NULL),
    m_frameBuffer(NULL),
    m_FadeinTimeMs(0),
    m_FadeinTimeProgressMs(0),
    m_threadId(0),
    m_muteFlag(false),
    m_stopFlag(false),
    m_playReadyFlag(false),
    m_exitFlag(false),
    m_currPlayType(FilePlayType::FILE_PLAYTYPE_PROMPT) {
    m_executor = ThreadPoolWrapper::getInstance();
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_playCond, NULL);

    m_alertsAlsa = new AlsaController(alertsAudioDev);
    m_alertsAlsa->init(OUTPUT_SAMPLE_RATE, OUTPUT_CHANNEL);
    m_promptAlsa = new AlsaController(promptAudioDev);
    m_promptAlsa->init(OUTPUT_SAMPLE_RATE, OUTPUT_CHANNEL);

    m_audioDecoder = new AudioDecoder(OUTPUT_CHANNEL, OUTPUT_SAMPLE_RATE);
    m_pcmBuffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE);
    m_frameBuffer = (uint8_t*)av_malloc(FRAME_BUFFER_SIZE);

    pthread_create(&m_threadId, NULL, playFunc, this);
    deviceCommonLib::deviceTools::setThreadName(m_threadId, "PLY_fileplay");
}

Mp3FilePlayerImpl::~Mp3FilePlayerImpl() {
    m_stopFlag = true;
    m_exitFlag = true;
    pthread_cond_signal(&m_playCond);

    void* thread_return = NULL;

    if (m_threadId != 0) {
        pthread_join(m_threadId, &thread_return);
    }

    m_alertsAlsa->aslaAbort();
    m_alertsAlsa->alsaClose();
    if (m_alertsAlsa) {
        delete m_alertsAlsa;
        m_alertsAlsa = NULL;
    }

    m_promptAlsa->aslaAbort();
    m_promptAlsa->alsaClose();
    if (m_promptAlsa) {
        delete m_promptAlsa;
        m_promptAlsa = NULL;
    }

    if (m_pcmBuffer != NULL) {
        av_free(m_pcmBuffer);
        m_pcmBuffer = NULL;
    }

    if (m_frameBuffer != NULL) {
        av_free(m_frameBuffer);
        m_frameBuffer = NULL;
    }

    if (m_audioDecoder != NULL) {
        delete m_audioDecoder;
        m_audioDecoder = NULL;
    }

    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_playCond);
}

void* Mp3FilePlayerImpl::playFunc(void* arg) {
    Mp3FilePlayerImpl* player = (Mp3FilePlayerImpl*) arg;

    while (!player->m_exitFlag) {
        pthread_mutex_lock(&player->m_mutex);
        player->m_playReadyFlag = true;

        while (player->m_resPath.empty()) {
            pthread_cond_wait(&player->m_playCond, &player->m_mutex);

            if (player->m_exitFlag) {
                return NULL;
            }
        }

        player->executePlaybackStarted();

        APP_INFO("play begin:%s", player->m_resPath.c_str());
		inf("play begin:%s", player->m_resPath.c_str());
        AlsaController *alsaPtr = NULL;
        if (player->m_currPlayType == FilePlayType::FILE_PLAYTYPE_PROMPT) {
            alsaPtr = player->m_promptAlsa;
        } else {
            alsaPtr = player->m_alertsAlsa;
        }
        alsaPtr->alsaPrepare();

        bool validUrl = player->m_audioDecoder->open(player->m_resPath);

        if (!validUrl) {
            APP_ERROR("play cancel, invalid url");
        }

        int pos = 0;
        player->fadeinReset();

        while (!player->m_stopFlag && validUrl) {
            if (player->m_muteFlag) {
                usleep(300000);
                continue;
            }

            unsigned int len = 0;
            memset(player->m_frameBuffer, 0, FRAME_BUFFER_SIZE);
            StreamReadResult ret = player->m_audioDecoder->readFrame(&player->m_frameBuffer,
                                   FRAME_BUFFER_SIZE,
                                   &len);

            if (ret == READ_SUCCEED) {
                uint8_t* p = player->m_pcmBuffer + pos;

                if (pos + len > MAX_AUDIO_FRAME_SIZE) {
                    player->fadeinProcess(pos);
                    alsaPtr->writeStream(OUTPUT_CHANNEL,
                                         player->m_pcmBuffer,
                                         pos);
                    pos = 0;
                    p = player->m_pcmBuffer;
                    memcpy(p, player->m_frameBuffer, len);
                    pos += len;
                } else {
                    memcpy(p, player->m_frameBuffer, len);
                    pos += len;
                }
            } else {
                if (ret == READ_END) {
                    break;
                } else {
                    continue;
                }
            }
        }

        if (pos > 0) {
            player->fadeinProcess(pos);
            alsaPtr->writeStream(OUTPUT_CHANNEL,
                                 player->m_pcmBuffer,
                                 pos);
        }

        player->m_audioDecoder->close();
        usleep(500000);
        player->executePlaybackFinished();
        APP_INFO("play end, wait for next");

        alsaPtr->alsaClear();
        player->m_resPath = "";
        player->m_playReadyFlag = false;
        pthread_mutex_unlock(&player->m_mutex);
    }

    return NULL;
}

void Mp3FilePlayerImpl::registerListener(std::shared_ptr<AudioPlayerListener> notify) {
}

void Mp3FilePlayerImpl::registerListener(std::shared_ptr<AudioPlayerListener> notify, FilePlayType type) {
    if (type == FILE_PLAYTYPE_ALERTS) {
        m_alertsListener = notify;
    }
    if (type == FILE_PLAYTYPE_PROMPT) {
        m_promptListener = notify;
    }
}

void Mp3FilePlayerImpl::setFilePlayType(FilePlayType type) {
    if (type == FILE_PLAYTYPE_PROMPT) {
        m_currentListener = m_promptListener;
    }
    if (type == FILE_PLAYTYPE_ALERTS) {
        m_currentListener = m_alertsListener;
    }
    m_currPlayType = type;
    APP_INFO("current playing type:%d", type);
}

void Mp3FilePlayerImpl::setPlayMode(PlayMode mode, unsigned int val) {
    if (m_audioDecoder != NULL) {
        if (mode == PLAYMODE_LOOP) {
            m_audioDecoder->setDecodeMode(DECODE_MODE_LOOP, val);
        } else {
            m_audioDecoder->setDecodeMode(DECODE_MODE_NORMAL, 0);
        }
    }
}

void Mp3FilePlayerImpl::audioPlay(const std::string& url,
                                  unsigned int offset,
                                  unsigned int report_interval) {
    APP_INFO("audio_play called enter");

    while (!m_playReadyFlag) {
        usleep(20000);
    }

    audioStop();
    pthread_mutex_lock(&m_mutex);
    m_resPath = url;
    m_stopFlag = false;
    pthread_cond_signal(&m_playCond);
    pthread_mutex_unlock(&m_mutex);
    APP_INFO("audio_play called exit");
}

void Mp3FilePlayerImpl::audioPause() {
	APP_INFO(__func__);
}

void Mp3FilePlayerImpl::audioResume() {
	APP_INFO(__func__);
}

void Mp3FilePlayerImpl::audioStop() {
    APP_INFO("audio_stop called enter");
    m_stopFlag = true;
    pthread_mutex_lock(&m_mutex);
    pthread_mutex_unlock(&m_mutex);
    if (m_currPlayType == FilePlayType::FILE_PLAYTYPE_PROMPT) {
        m_promptAlsa->alsaClear();
    } else {
        m_alertsAlsa->alsaClear();
    }
    APP_INFO("audio_stop called exit");
}

void Mp3FilePlayerImpl::setMute() {
    m_muteFlag = true;
}

void Mp3FilePlayerImpl::setUnmute() {
    m_muteFlag = false;
}

void Mp3FilePlayerImpl::setFadeIn(int timeSec) {
    APP_INFO("setFadeIn %d", timeSec);
    m_FadeinTimeMs = timeSec * 1000;
    return;
}

void Mp3FilePlayerImpl::executePlaybackStarted() {
    if (m_currentListener == NULL) {
        return;
    }

    APP_INFO("executePlaybackStarted called");
    m_executor->submit([this]() {
        m_currentListener->playbackStarted(0);
    });
}

void Mp3FilePlayerImpl::executePlaybackFinished() {
    if (m_currentListener == NULL) {
        return;
    }

    APP_INFO("executePlaybackFinished called");
    m_executor->submit([this]() {
        if (m_stopFlag) {
            m_currentListener->playbackFinished(0, AudioPlayerFinishStatus::AUDIOPLAYER_FINISHSTATUS_STOP);
        } else {
            m_currentListener->playbackFinished(0, AudioPlayerFinishStatus::AUDIOPLAYER_FINISHSTATUS_END);
        }
    });
}

float Mp3FilePlayerImpl::getFadeAttenuate() {
    float min_attenuate_in_db = 0;
    float max_attenuate_in_db = -80;
    float attenuate_in_db = max_attenuate_in_db + (min_attenuate_in_db - max_attenuate_in_db)  *
                            m_FadeinTimeProgressMs / m_FadeinTimeMs;

    if (attenuate_in_db > 0) {
        attenuate_in_db = 0;
    }

    float attenuate = exp(attenuate_in_db / 20);
    APP_INFO("atten_in_db %f atten %f", attenuate_in_db, attenuate);
    return attenuate;
}
void Mp3FilePlayerImpl::fadeinReset() {
    m_FadeinTimeProgressMs = 0;
}
void Mp3FilePlayerImpl::fadeinProcess(int  size) {
    if (m_FadeinTimeMs > m_FadeinTimeProgressMs) {
        //liner fade from 0-10 sec
        float attenuate = getFadeAttenuate();
        short val;

        for (int i = 0; i < size / 2; i++) {
            val = ((short*)m_pcmBuffer)[i] * attenuate;
            ((short*)m_pcmBuffer)[i] = val;
        }

        m_FadeinTimeProgressMs += size * 1000 / OUTPUT_SAMPLE_RATE / OUTPUT_CHANNEL / 2 ;
    }
}

unsigned long Mp3FilePlayerImpl::getProgress() {
    return 0;
}

unsigned long Mp3FilePlayerImpl::getDuration() {
    return 0;
}

}  // mediaPlayer
}  // duerOSDcsApp
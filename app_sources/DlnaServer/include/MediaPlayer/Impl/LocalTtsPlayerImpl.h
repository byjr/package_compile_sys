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

#ifndef DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_LOCALTTSPLAYERIMPL_H_
#define DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_LOCALTTSPLAYERIMPL_H_

#include <string>
#include <unistd.h>
//#include "TTSManager/TTSManager.h"
#include "StreamPool.h"
#include "AlsaController.h"
#include "PcmResampler.h"
#include "LocalTtsPlayerInterface.h"
#include "DCSApp/ThreadPoolWrapper.h"
#include "PthreadLock.h"
#include "AutoLock.h"
#include <libcchip/platform.h>
#include <curl/curl.h>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"

namespace duerOSDcsApp {
namespace mediaPlayer {

//using duerOSDcsSDK::ttsManager::TTSManager;
//using duerOSDcsSDK::ttsManager::TTSSynthesizeResultObserver;
using application::ThreadPoolWrapper;

class LocalTtsPlayerImpl : public LocalTtsPlayerInterface {
public:
    static std::shared_ptr<LocalTtsPlayerImpl> create(const std::string& audio_dev,
            const std::string& res_path);

    virtual ~LocalTtsPlayerImpl();

    void ttsPlay(const std::string& content) override;

    void ttsStop() override;

    void registerListener(std::shared_ptr<LocalTtsPlayerListener> notify) override;

private:
    LocalTtsPlayerImpl(const std::string& audio_dev,
                       const std::string& synthesize_res_path);

    LocalTtsPlayerImpl(const LocalTtsPlayerImpl&);

    LocalTtsPlayerImpl& operator=(const LocalTtsPlayerImpl&);

    TtsPlayerStatus getStatus();


    void setStatus(TtsPlayerStatus status);

    void executeSpeechStarted();

    void executeSpeechFinished();

    void ttsSynthesizeBegin(const std::string& strIndex);

    void ttsSynthesizeAudioData(const char* stream,
                                unsigned int length,
                                unsigned int charIndex,
                                const std::string& strIndex = "");

    void ttsSynthesizeFinish(const std::string& strIndex = "");

    void ttsSynthesizeFailed(int errCode,
                             const std::string& errDesc,
                             const std::string& strIndex = "");
	static ssize_t writefunc_data(void *dat, size_t size, size_t nmemb, LocalTtsPlayerImpl *ptr);
    static void* playFunc(void* arg);
	std::string speechGetToken();
	bool speechTxtGet();
private:
    ThreadPoolWrapper* m_executor;
	AlsaController* m_alsaCtl;
    std::vector<std::shared_ptr<LocalTtsPlayerListener> > m_listeners;
    pthread_t m_playThread;
    pthread_mutex_t m_playMutex;
    pthread_cond_t m_playCond;
    volatile TtsPlayerStatus m_status;
    PthreadLock* m_lock;
    std::string m_ttsContent;
    bool m_firstPackFlag;
	bool m_aTokenInvaildFlag;
    bool m_threadAlive;
    bool m_pushStreamFlag;
    unsigned int m_maxValidLen;
    char *api_key;
    char *secret_key;
    char *cuid;
    int spd;
    int pit;
    int vol;
    int per;
	int aue;
	std::string aToken;
	autom_t *_autom;
	CURL *_curl;

};

}  // mediaPlayer
}  // duerOSDcsApp

#endif  // DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_LOCALTTSPLAYERIMPL_H_

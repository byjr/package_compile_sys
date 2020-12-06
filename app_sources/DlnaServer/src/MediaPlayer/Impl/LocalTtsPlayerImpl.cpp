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

#include "LocalTtsPlayerImpl.h"
#include "DCSApp/Configuration.h"
#include "DeviceTools/Utils.h"
namespace duerOSDcsApp {
namespace mediaPlayer {
#define TTS_PCM_RATE     16000
#define TTS_PCM_CHANNEL  1
#define SPEECHTXT_PLAY_PERIOS_BYTES 320
#define TEXT_TO_AUDIO_PATH "./t2a_token.txt"
extern "C"{
static ssize_t speechGetTokenWriteFunc(void *ptr, size_t size, size_t nmemb, autom_t * _autom) {
	size_t result_len = autom_write(_autom,(char *)ptr,size * nmemb);
	if(result_len < 0){
		err("");
		return -1;
	}
	return nmemb;	
}
}

std::string LocalTtsPlayerImpl::speechGetToken(){
	std::string url = "http://openapi.baidu.com/oauth/2.0/token?grant_type=client_credentials";
	url += "&client_id=";
	url += api_key;
	url += "&client_secret=";
	url += secret_key;
	autom_t * _autom = autom_create(NULL,1024);
	if(!_autom){
		return "";
	}
    CURL *curl = curl_easy_init();
	if(!curl){
		autom_destroy(_autom);
		return "";
	}inf("%s:url:%s",__func__,url.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // 注意返回值判读
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60); // 60s超时
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, speechGetTokenWriteFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, _autom);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    CURLcode res_curl = curl_easy_perform(curl);
	if (res_curl != CURLE_OK) {
		return "";
	}
	std::string json_string = _autom->head;
	autom_destroy(_autom);
	curl_easy_cleanup(curl);
	rapidjson::Document root;
	root.Parse<0>(json_string.c_str());
	if(root.HasParseError()){
		return "";
	}
	if(!root.IsObject()){
		return "";
	}
	if(!root.HasMember("scope")){
		return "";
	}
	std::string scope = root["scope"].GetString();
	if(!strstr(scope.c_str(),"audio_tts_post")){
		return "";
	}	
	if( !root.HasMember("access_token")){
		return "";
	}
	std::string aToken = root["access_token"].GetString();
	if(aToken.empty()){
		return "";
	}
	std::ofstream t2aToken(TEXT_TO_AUDIO_PATH);
	t2aToken << aToken;
	t2aToken.close();
	return root["access_token"].GetString();
}
ssize_t LocalTtsPlayerImpl::writefunc_data(void *dat, size_t size, size_t nmemb, LocalTtsPlayerImpl *ptr) {
	if(ptr->m_firstPackFlag){
		ptr->m_firstPackFlag = false;
		rapidjson::Document ctxRoot;
		ctxRoot.Parse<0>((const char *)dat);
		if(!ctxRoot.HasParseError()){
			err("%s:%s",__func__,dat);
			ptr->m_aTokenInvaildFlag = true;
			return -1;
		}		
	}
	if(ptr->m_status != TtsPlayerStatusPlay){
		return 0;
	}
	ssize_t res = autom_write(ptr->_autom,(char*)dat,size*nmemb);
	if(res < 0){
		err("");
		return -1;
	}
	return nmemb;
}
bool LocalTtsPlayerImpl::speechTxtGet() {
	if(aToken.empty()){
		err("speechGetToken failed!");
		return false;
	}
	char *textemp = curl_escape(m_ttsContent.c_str(),m_ttsContent.length());
	char *tex = curl_escape(textemp, strlen(textemp));
	curl_free(textemp);
	std::stringstream params;
	params << "ctp=1&lan=zh&cuid=" << cuid << "&tok=" << aToken.c_str();
	params  << "&tex=" << tex << "&per=" << per << "&spd=" << spd << "&pit=" << pit << "&vol=" << vol << "&aue=" << aue;
	curl_free(tex);
	_curl = curl_easy_init();
	if(!_curl){
		return false;
	}
	std::string url("http://tsn.baidu.com/text2audio?");
	url += params.str();dbg("url:%s",url.c_str());
	curl_easy_setopt(_curl,CURLOPT_URL,url.c_str());
	curl_easy_setopt(_curl,CURLOPT_CONNECTTIMEOUT, 5);
	curl_easy_setopt(_curl,CURLOPT_TIMEOUT, 60);
	_autom = autom_create(NULL,320*1000);
	if(!_autom){
		return false;
	}
	curl_easy_setopt(_curl,CURLOPT_WRITEFUNCTION, writefunc_data);
	curl_easy_setopt(_curl,CURLOPT_WRITEDATA,this);
	curl_easy_setopt(_curl,CURLOPT_VERBOSE, 0L);
	CURLcode res = curl_easy_perform(_curl);
	curl_easy_cleanup(_curl);
	if (res != CURLE_OK) {
		err("perform _curl error:%d[%s]", (int)res,curl_easy_strerror(res));
		return false;
	}else{
		return true;		
	}
}

std::shared_ptr<LocalTtsPlayerImpl> LocalTtsPlayerImpl::create(const std::string& audio_dev,
        const std::string& res_path) {
    std::shared_ptr<LocalTtsPlayerImpl> player(new LocalTtsPlayerImpl(audio_dev, res_path));
    return player;
}

LocalTtsPlayerImpl::LocalTtsPlayerImpl(const std::string& audio_dev,
                                       const std::string& synthesize_res_path): m_executor(NULL),
//                                                                                m_ttsMgr(NULL),
                                                                                m_alsaCtl(NULL),
                                                                                m_lock(NULL),
                                                                                m_status(TtsPlayerStatusIdle),
                                                                                m_firstPackFlag(false),
                                                                                m_threadAlive(true),
                                                                                m_pushStreamFlag(false) {
    m_executor = ThreadPoolWrapper::getInstance();
//    TTSManager::loadTtsEngine(synthesize_res_path);
//    m_ttsMgr = TTSManager::getTtsManagerInstance();
//    m_ttsMgr->setTtsSynthesizeResultObserver(this);
    m_alsaCtl = new AlsaController(audio_dev);
    m_alsaCtl->init(TTS_PCM_RATE, TTS_PCM_CHANNEL);
    m_lock = new PthreadLock();
    pthread_mutex_init(&m_playMutex, NULL);
    pthread_cond_init(&m_playCond, NULL);
    m_maxValidLen = 4294967295 / 3;

	api_key = "4E1BG9lTnlSeIf1NQFlrSq6h";
	secret_key = "544ca4657ba8002e3dea3ac2f5fdd241";
	cuid = "1234567C";
	per = 0;//0为普通女声，1为普通男生，3为情感合成-度逍遥，4为情感合成-度丫丫，默认为普通女声
	spd = 6;// 语速，取值0-9，默认为5中语速
	pit = 5;// #音调，取值0-9，默认为5中语调
	vol = 5;// #音量，取值0-9，默认为5中音量
	aue = 4;// 下载的文件格式, 3：mp3	4：pcm-16k  5：pcm-8k  6. wav
	m_aTokenInvaildFlag = false;
	std::ifstream t2aToken(TEXT_TO_AUDIO_PATH);
	if(!t2aToken){
		aToken = speechGetToken();
	}else{
		t2aToken >> aToken;
		t2aToken.close();
	}
	inf("aToken:%s",aToken.c_str());
    pthread_create(&m_playThread, NULL, playFunc, this);
    deviceCommonLib::deviceTools::setThreadName(m_playThread, "PLY_localtts");
}

LocalTtsPlayerImpl::~LocalTtsPlayerImpl() {
    m_threadAlive = false;
    pthread_cond_signal(&m_playCond);
    void* play_thread_return = NULL;

    if (m_playThread != 0) {
        pthread_join(m_playThread, &play_thread_return);
    }

    if (m_lock) {
        delete m_lock;
        m_lock = NULL;
    }
    pthread_mutex_destroy(&m_playMutex);
    pthread_cond_destroy(&m_playCond);
    m_alsaCtl->alsaClear();
    m_alsaCtl->alsaClose();
}

void LocalTtsPlayerImpl::registerListener(std::shared_ptr<LocalTtsPlayerListener> notify) {
    if (notify) {
        m_listeners.push_back(notify);
    }
}

void LocalTtsPlayerImpl::ttsPlay(const std::string& content) {
	m_status = TtsPlayerStatusStop;
	pthread_mutex_lock(&m_playMutex);
	m_ttsContent = content;
	m_firstPackFlag = true;
	m_status = TtsPlayerStatusPlay;
	pthread_mutex_unlock(&m_playMutex);
    m_executor->submit([this]() {
        if (m_alsaCtl->isAccessable()) {           
            m_alsaCtl->alsaClear();
            m_alsaCtl->alsaPrepare();
			pthread_mutex_lock(&m_playMutex);
			pthread_cond_signal(&m_playCond);
			pthread_mutex_unlock(&m_playMutex);			
        }
    });
}

void LocalTtsPlayerImpl::ttsStop() {
    if (m_status != TtsPlayerStatusPlay) {
        return;
    }
    m_executor->submit([this]() {
		m_status = TtsPlayerStatusStop;
//        m_ttsMgr->stopSynthesize();
        m_alsaCtl->alsaClear();
//        m_streamPool.clearItems();
    });
}

void* LocalTtsPlayerImpl::playFunc(void* arg) {
    LocalTtsPlayerImpl* tts_player = (LocalTtsPlayerImpl*)arg;

    while (tts_player->m_threadAlive) {
        pthread_mutex_lock(&tts_player->m_playMutex);
		pthread_cond_wait(&tts_player->m_playCond, &tts_player->m_playMutex);
		if(tts_player->speechTxtGet() == false){
			if(tts_player->m_aTokenInvaildFlag){
				tts_player->aToken = tts_player->speechGetToken();
				if(tts_player->speechTxtGet() == false){
					err("");
					tts_player->m_status = TtsPlayerStatusStop;
				}
			}
		}
		tts_player->executeSpeechStarted();
		for(;tts_player->m_status == TtsPlayerStatusPlay;){
			char *ri = autom_read(tts_player->_autom,SPEECHTXT_PLAY_PERIOS_BYTES);
			size_t play_bytes = SPEECHTXT_PLAY_PERIOS_BYTES;
			if(ri){
				tts_player->m_alsaCtl->writeStream(TTS_PCM_CHANNEL,ri,SPEECHTXT_PLAY_PERIOS_BYTES);
			}else{
				play_bytes = tts_player->_autom->cur_bytes;
				ri = tts_player->_autom->tail - tts_player->_autom->cur_bytes;
				tts_player->m_alsaCtl->writeStream(TTS_PCM_CHANNEL,ri,play_bytes);
				break;
			}	
		}
		autom_destroy(tts_player->_autom);
        tts_player->m_alsaCtl->alsaClear();
		tts_player->m_status = TtsPlayerStatusStop;
        tts_player->executeSpeechFinished();
        pthread_mutex_unlock(&tts_player->m_playMutex);
    }

    return NULL;
}

void LocalTtsPlayerImpl::ttsSynthesizeBegin(const std::string& strIndex) {
//    if (m_status != TtsPlayerStatusPlay) {
//        return;
//    }
//
//    m_firstPackFlag = true;
//    m_pushStreamFlag = true;
//    pthread_cond_signal(&m_playCond);
//    executeSpeechStarted();
}

void LocalTtsPlayerImpl::ttsSynthesizeAudioData(const char* stream,
        unsigned int length,
        unsigned int charIndex,
        const std::string& strIndex) {
//    if (getStatus() != TtsPlayerStatusPlay) {
//        return;
//    }
//
//    if (m_firstPackFlag) {
//        m_firstPackFlag = false;
//    }
//
//    if (length > m_maxValidLen) {
//    } else {
//        unsigned long resample_len = 3 * length;
//        uint8_t* resample_data = (uint8_t*)malloc(resample_len);
//        PcmResampler::getInstance()->pcmResample(&resample_data, resample_len / 2,
//                (const uint8_t**)(&stream), length / 2);
//        m_streamPool.pushStream((char*)resample_data, resample_len);
//        free(resample_data);
//    }
}

void LocalTtsPlayerImpl::ttsSynthesizeFinish(const std::string& strIndex) {
//    if (m_status != TtsPlayerStatusPlay) {
//        return;
//    }
//
//    m_pushStreamFlag = false;
}

void LocalTtsPlayerImpl::ttsSynthesizeFailed(int errCode,
        const std::string& errDesc,
        const std::string& strIndex) {
}

void LocalTtsPlayerImpl::executeSpeechStarted() {
    m_executor->submit([this]() {
        size_t len = m_listeners.size();

        for (size_t i = 0; i < len; ++i) {
            if (NULL != m_listeners[i]) {
                m_listeners[i]->speechStarted();
            }
        }
    });
}

void LocalTtsPlayerImpl::executeSpeechFinished() {
    m_executor->submit([this]() {
        size_t len = m_listeners.size();
        for (size_t i = 0; i < len; ++i) {
            if (NULL != m_listeners[i]) {
                m_listeners[i]->speechFinished();
            }
        }
    });
}

}  // mediaPlayer
}  // duerOSDcsApp

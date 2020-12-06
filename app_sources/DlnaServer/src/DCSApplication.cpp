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

#include "DCSApp/DCSApplication.h"
#include <algorithm>
#include <fstream>
#include "DCSApp/RecordAudioManager.h"
#include "DCSApp/ApplicationManager.h"
#ifdef KITTAI_KEY_WORD_DETECTOR
#include "DCSApp/VoiceAndTouchWakeUpObserver.h"
#endif
#include "DCSApp/Configuration.h"
#include "TtsPlayerProxy.h"
#include "MediaPlayerProxy.h"
#include "AlertsPlayerProxy.h"
#include "LocalPlayerProxy.h"
#include "BluetoothPlayerImpl.h"
#include "Mp3UrlPlayerImpl.h"
#include "Mp3TtsPlayerImpl.h"
#include "Mp3FilePlayerImpl.h"
#include "FilePlayerSingletonProxy.h"
#include "PcmTtsPlayerImpl.h"
#ifdef LOCALTTS
#include "LocalTtsProxy.h"
#include "LocalTtsPlayerImpl.h"
#endif
#include "BlueToothPlayerProxy.h"
#include "LoggerUtils/DcsSdkLogger.h"
#include "DCSApp/ActivityMonitorSingleton.h"

#ifdef DUEROS_DLNA
#include "Dlna/DlnaDmrSdk.h"
#include "Dlna/DlnaDmrOutputFfmpeg.h"
#include "DlnaPlayerImpl.h"
#endif
#include "DCSApp/Audio/AlsaAudioMicrophoneWrapper.h"
#include "DcsSdk/ApplicationImplementation.h"
#include "DcsSdk/AudioCallInterface.h"
#include "DCSApp/AudioCallProxy.h"

typedef std::shared_ptr<duerOSDcsApp::application::AudioMicrophoneInterface> createT(std::shared_ptr<duerOSDcsSDK::sdkInterfaces::DcsSdk> dcsSdk);
namespace duerOSDcsApp {
namespace application {

#ifdef MTK8516
#define PCM_DEVICE "track:10"
#else
#define PCM_DEVICE "default"
#endif

/// The config path of DCSSDK
static const std::string PATH_TO_CONFIG = "./resources/config.json";

#ifdef MTK8516
/// The runtime config path of DCSSDK
static const std::string PATH_TO_RUNTIME_CONFIG = "/data/duer/runtime.json";

static const std::string PATH_TO_BDUSS_FILE = "/data/duer/bduss.txt";
#else
/// The runtime config path of DCSSDK
static const std::string PATH_TO_RUNTIME_CONFIG = "./resources/runtime.json";

static const std::string PATH_TO_BDUSS_FILE = "./bduss.txt";
#endif

std::unique_ptr<DCSApplication> DCSApplication::create() {
    auto dcsApplication = std::unique_ptr<DCSApplication>(new DCSApplication);
    if (!dcsApplication->initialize()) {
        APP_ERROR("Failed to initialize SampleApplication");
        return nullptr;
    }
    return dcsApplication;
}

void DCSApplication::run() {
//	DuerosFifoCmd::getInstance()->DuerosFifoCmdProc(NULL);
//    while(true) {
//        struct timeval wait = {0, 100 * 1000};
//        ::select(0, NULL, NULL, NULL, &wait);
//    }

//	  while(1){pause();}
	  duerOSDcsApp::voip::AudioCallProxy::tipSoundAudio(NULL);
}

bool DCSApplication::initialize() {
    m_audioDyLib = std::make_shared<deviceCommonLib::deviceTools::DyLibrary>();
    DeviceIoWrapper::getInstance()->initCommonVolume(Configuration::getInstance()->getCommVol());
    DeviceIoWrapper::getInstance()->initAlertVolume(Configuration::getInstance()->getAlertsVol());
    /*
     * Creating the media players.
     */
    Configuration *configuration = Configuration::getInstance();
	if(!configuration){
		APP_ERROR("Failed to create Configuration!");
		return false;
	}

	auto m_AudioCall = duerOSDcsApp::voip::AudioCallProxy::getInstance();
	if(!m_AudioCall){
		APP_ERROR("Failed to create AudioCallInterface!");
		return false;
	}

    auto speakMediaPlayer = mediaPlayer::TtsPlayerProxy::create(configuration->getTtsPlaybackDevice());
    if (!speakMediaPlayer) {
        APP_ERROR("Failed to create media player for speech!");
        return false;
    }
	

    auto audioMediaPlayer = mediaPlayer::MediaPlayerProxy::create(configuration->getMusicPlaybackDevice());
    if (!audioMediaPlayer) {
        APP_ERROR("Failed to create media player for content!");
        return false;
    }
	

    auto alertsMediaPlayer = mediaPlayer::AlertsPlayerProxy::create(configuration->getAlertPlaybackDevice());
    if (!alertsMediaPlayer) {
        APP_ERROR("Failed to create media player for alerts!");
        return false;
    }
    //alertsMediaPlayer->setFadeIn(10);
	
    auto localPromptPlayer = mediaPlayer::LocalPlayerProxy::create(configuration->getInfoPlaybackDevice());
    if (!localPromptPlayer) {
        APP_ERROR("Failed to create media player for local!");
        return false;
    }

#ifdef LOCALTTS
    auto localTtsPlayer = mediaPlayer::LocalTtsProxy::create(configuration->getNattsPlaybackDevice());
    if (!localTtsPlayer) {
        APP_ERROR("Failed to create media player for local tts!");
        return false;
    }
#endif
	

    auto blueToothPlayer = mediaPlayer::BlueToothPlayerProxy::create();
    if (!blueToothPlayer) {
        APP_ERROR("Failed to create blueToothPlayer!");
        return false;
    }

    auto applicationManager = std::make_shared<ApplicationManager>();
	
    duerOSDcsSDK::sdkInterfaces::DcsSdkParameters parameters;
    parameters.setPathToConfig(PATH_TO_CONFIG);
    parameters.setPathToRuntimeConfig(PATH_TO_RUNTIME_CONFIG);
    parameters.setDeviceId(DeviceIoWrapper::getInstance()->getDeviceId());
	parameters.setAudioCall((std::shared_ptr<AudioCallInterface>)m_AudioCall);
    parameters.setSpeakMediaPlayer(speakMediaPlayer);
    parameters.setAudioMediaPlayer(audioMediaPlayer);
    parameters.setAlertsMediaPlayer(alertsMediaPlayer);
    parameters.setLocalPromptPlayer(localPromptPlayer);
    parameters.setDialogStateObservers({applicationManager});
    parameters.setConnectionObservers(applicationManager);
    parameters.setApplicationImplementation(applicationManager);
    parameters.setLocalMediaPlayer(blueToothPlayer);

	std::string CmccVoipNamespace = "ai.dueros.device_interface.extensions.telephone";
	parameters.getCustomizeDirective().emplace_back(std::make_pair(CmccVoipNamespace, "PhonecallByName"));
	parameters.getCustomizeDirective().emplace_back(std::make_pair(CmccVoipNamespace, "PhonecallByNumber"));
	parameters.getCustomizeDirective().emplace_back(std::make_pair(CmccVoipNamespace, "SelectCallee"));

	std::string ThirdPartyNamespace = "ai.dueros.device_interface.thirdparty.cmcc.call";
	parameters.getCustomizeDirective().emplace_back(std::make_pair(ThirdPartyNamespace, "Hangup"));
	parameters.getCustomizeDirective().emplace_back(std::make_pair(ThirdPartyNamespace, "Receive"));
	parameters.getCustomizeDirective().emplace_back(std::make_pair(ThirdPartyNamespace, "Upload_log"));
	parameters.getCustomizeDirective().emplace_back(std::make_pair(ThirdPartyNamespace, "Qry_vol"));
	parameters.getCustomizeDirective().emplace_back(std::make_pair(ThirdPartyNamespace, "Qry_ver"));
	parameters.getCustomizeDirective().emplace_back(std::make_pair(ThirdPartyNamespace, "Upgrade_ver"));
	
    parameters.setDebugFlag(Configuration::getInstance()->getDebug());
	inf("getDebugFlag:%d",parameters.getDebugFlag()?1:0);
		
#ifdef KITTAI_KEY_WORD_DETECTOR
		// This observer is notified any time a keyword is detected and notifies the DCSSDK to start recognizing.
		auto voiceAndTouchWakeUpObserver = std::make_shared<VoiceAndTouchWakeUpObserver>();
		parameters.setKeyWordObserverInterface(voiceAndTouchWakeUpObserver);
#endif
		
#ifdef LOCALTTS
		parameters.setLocalTtsMediaPlayer(localTtsPlayer);
#endif
		
		m_dcsSdk = duerOSDcsSDK::sdkInterfaces::DcsSdk::create(parameters);		
		if (!m_dcsSdk) {
			APP_ERROR("Failed to create default SDK handler!");
			return false;
		}
		
	//	  m_audioLibName = Configuration::getInstance()->getAudioLibName();
	//	inf("");
	//	  bool ret = m_audioDyLib->load(m_audioLibName);inf("");
	//	  if (!ret) {
	//		  LOGGER_ERROR(LX("initialize Failed").d("reason", "load library error"));
	//		  return false;
	//	  }
	//	  createT * m_create = (createT *)m_audioDyLib->getSym("create");
	//	inf("");
	//	  if (!m_create) {
	//		  LOGGER_ERROR(LX("initialize Failed").d("reason", "get symbol error"));
	//		  return false;
	//	  }
		
		std::shared_ptr<AudioMicrophoneInterface>  micWrapper = AlsaAudioMicroponeWrapper::create(m_dcsSdk);
		if(!micWrapper.get()){
			err(" AlsaAudioMicroponeWrapper::create failed!");
			return false;
		}
//		micWrapper->setRecordDataInputCallback(recordDataInputCallback);
		
		applicationManager->setMicrophoneWrapper(micWrapper);
		applicationManager->setDcsSdk(m_dcsSdk);
		blueToothPlayer->setDcsSdk(m_dcsSdk);
		
		//create player client and attach to the player proxy. you may replace playerclient with your own
		auto mp3UrlPlayerImpl = mediaPlayer::Mp3UrlPlayerImpl::create(configuration->getMusicPlaybackDevice());
		mp3UrlPlayerImpl->registerListener(audioMediaPlayer);
		audioMediaPlayer->setImpl(mp3UrlPlayerImpl);
		
		auto mp3TtsPlayerImpl = mediaPlayer::Mp3TtsPlayerImpl::create(configuration->getTtsPlaybackDevice());
		mp3TtsPlayerImpl->registerListener(speakMediaPlayer);
		speakMediaPlayer->setMp3TtsImpl(mp3TtsPlayerImpl);
		
		auto pcmTtsPlayerImpl = mediaPlayer::PcmTtsPlayerImpl::create(PCM_DEVICE);
		pcmTtsPlayerImpl->registerListener(speakMediaPlayer);
		speakMediaPlayer->setPcmTtsImpl(pcmTtsPlayerImpl);
		
		mediaPlayer::FilePlayerSingletonProxy* filePlayerSingleton = mediaPlayer::FilePlayerSingletonProxy::getInstance();
		std::shared_ptr<mediaPlayer::FilePlayerSingletonProxy> playerSingleton(filePlayerSingleton);
		const std::string& alertsDev = configuration->getAlertPlaybackDevice();
		const std::string& promptDev = configuration->getInfoPlaybackDevice();
		filePlayerSingleton->initialiaze(alertsDev, promptDev);
		filePlayerSingleton->registerListener(alertsMediaPlayer, mediaPlayer::FILE_PLAYTYPE_ALERTS);
		alertsMediaPlayer->setImpl(playerSingleton);
		filePlayerSingleton->registerListener(localPromptPlayer, mediaPlayer::FILE_PLAYTYPE_PROMPT);
		localPromptPlayer->setImpl(playerSingleton);
		
#ifdef LOCALTTS
		auto localTtsPlayerImpl = mediaPlayer::LocalTtsPlayerImpl::create(configuration->getNattsPlaybackDevice(),
																			  "./resources/");
		localTtsPlayerImpl->registerListener(localTtsPlayer);
		localTtsPlayer->setImpl(localTtsPlayerImpl);
#endif
		
		auto btPlayerImpl = mediaPlayer::BluetoothPlayerImpl::create();
		btPlayerImpl->registerListener(blueToothPlayer);
		blueToothPlayer->setBluetoothImpl(btPlayerImpl);
		
#ifdef DUEROS_DLNA
		auto dlnaPlayerImpl = mediaPlayer::DlnaPlayerImpl::create();
		dlnaPlayerImpl->registerListener(blueToothPlayer);
		blueToothPlayer->setDlnaImpl(dlnaPlayerImpl);
#endif
		
#ifdef KITTAI_KEY_WORD_DETECTOR
		voiceAndTouchWakeUpObserver->setDcsSdk(m_dcsSdk);
#endif
				
#ifdef LOCALTTS
		SoundController::getInstance()->addObserver(localTtsPlayer);
#endif
		SoundController::getInstance()->addObserver(localPromptPlayer);
#ifdef KITTAI_KEY_WORD_DETECTOR
		DeviceIoWrapper::getInstance()->addObserver(voiceAndTouchWakeUpObserver);
#endif
		DeviceIoWrapper::getInstance()->addObserver(btPlayerImpl);
#ifdef DUEROS_DLNA
		DeviceIoWrapper::getInstance()->addObserver(dlnaPlayerImpl);
#endif
		DeviceIoWrapper::getInstance()->setApplicationManager(applicationManager);
//		applicationManager->microphoneOn();
		DeviceIoWrapper::getInstance()->volumeChanged();
		
		
		DuerLinkWrapper::getInstance()->setCallback(this);
		
		if (framework::DeviceIo::getInstance()->inOtaMode()) {
			DeviceIoWrapper::getInstance()->setMute(true);
		}
		
#if defined (MTK8516) || defined (CONFIG_PANTHER)
#ifdef DUERLINK_V2
		DuerLinkWrapper::getInstance()->initDuerLink();
#else
	//	std::string accessToken="";
	//	if(!ApplicationManager::globalGetAccessToken(accessToken)){
	//		err("getAccessqToken failed !");
	//		return false;
	//	}
	//	setAccessToken(accessToken);
	//	  DuerLinkWrapper::getInstance()->initDuerLink(accessToken, m_dcsSdk->getClientId(),ApplicationManager::globalRefreshAccessToken);
		DuerLinkWrapper::getInstance()->initDuerLink();
		
#endif
		DeviceIoWrapper::getInstance()->callback(duerOSDcsApp::framework::DeviceInput::KEY_MIC_SYNC,NULL,0);//开机同步麦克风状态
		DuerosFifoCmd *DuerosFifoCmdPtr = DuerosFifoCmd::getInstance();
		DuerosFifoCmdPtr->BluetoothPlayerImpl = btPlayerImpl.get();
		DuerosFifoCmd::getInstance()->startDuerosFifoCmdProc(NULL);
		DuerLinkWrapper::getInstance()->startNetworkRecovery();
		
		std::chrono::seconds secondsToWait{1};
		if (!m_detectNTPTimer.start(
				secondsToWait, deviceCommonLib::deviceTools::Timer::PeriodType::ABSOLUTE,
				deviceCommonLib::deviceTools::Timer::FOREVER,
				std::bind(&DCSApplication::detectNTPReady, this))) {
			APP_ERROR("Failed to start m_checkNtpTimer");
		} else {
			APP_INFO("succeed to start m_checkNtpTimer");
		}
#else
		m_dcsSdk->notifySystemTimeReady();
		m_dcsSdk->notifyNetworkReady(true, "testWifi");
#endif
		
#ifdef DUERLINK_V2
		DuerLinkWrapper::getInstance()->startDiscoverAndBound(m_dcsSdk->getClientId());
#else
	//	  DuerLinkWrapper::getInstance()->startDiscoverAndBound(m_dcsSdk->getClientId(), PATH_TO_BDUSS_FILE);
#endif
#ifdef DUEROS_DLNA
		std::shared_ptr<dueros_dlna::IOutput> sp = std::make_shared<dueros_dlna::OutputFfmpeg>();
		dueros_dlna::DlnaDmrSdk dlnaDmrSdk;
		dlnaDmrSdk.add_output_module(sp);
		dlnaDmrSdk.start();
#endif
		m_systemUpdateRevWrapper = SystemUpdateRevWrapper::create();
		
		return true;
	}

class platformNetworkTimeGet{
public:
	platformNetworkTimeGet(){
		timeGetStateCode = -1;
	}
	void runNetworkTimeGet(){
		CURL *curl = curl_easy_init(); // 需要释放
		if(!curl){
			return ;
		}
		curl_easy_setopt(curl, CURLOPT_URL, "www.baidu.com");
	    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5); // 连接5s超时
	    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30); // 整体请求60s超时	
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION,parseHeadCheck); // 检查头部
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
		CURLcode res_curl = curl_easy_perform(curl);
		if (res_curl != CURLE_OK && res_curl > 0) {
			err("err:%d[%s]", res_curl,curl_easy_strerror(res_curl));		
		}
		curl_easy_cleanup(curl);
	}
	char getTimeGetStateCode(){
		return timeGetStateCode;
	}
	static ssize_t parseHeadCheck(char *buffer, size_t uBs, size_t nus, platformNetworkTimeGet* ptr);	
private:
	char timeGetStateCode;
};
#define NETWORK_TIME_DEVIATION_SEC 2
ssize_t platformNetworkTimeGet::parseHeadCheck(char *buffer, size_t uBs, size_t nus, platformNetworkTimeGet* ptr) {
	if(!strstr(buffer,"Date:")){
		ptr->timeGetStateCode = -2;
		return uBs * nus;
	}
	inf("buffer:%s",buffer);
	struct tm newTm = {0};
	char * res = strptime(buffer,"Date: %a, %d %b %Y %T GMT",&newTm);
	if(!res){
		err("strptime");
		ptr->timeGetStateCode = -3;
		return -1;
	}
	struct timespec ts = {8*3600,500000000};	
	ts.tv_sec +=  mktime(&newTm);
	time_t newTime = ts.tv_sec;
	inf("newTime = %ld",newTime);
	time_t oldTime = 0;
	int ret = time(&oldTime);
	inf("oldTime = %ld",oldTime);	
	if( ret < 0 || (newTime>oldTime?newTime-oldTime:oldTime-newTime) > NETWORK_TIME_DEVIATION_SEC){
		inf("clock_settime ...");
		if(clock_settime(CLOCK_REALTIME,&ts)<0){
			ptr->timeGetStateCode = -4;
			return -1;
		}
	}
	ptr->timeGetStateCode = 0;
	return -1;
}

void DCSApplication::detectNTPReady() {
	static size_t detectNTPReadyCount = 0;
	SdkConnectionState m_sdkConnectionStates = m_dcsSdk->getSdkConnectionStates();
	if(m_sdkConnectionStates ==  SdkConnectionState::SDK_CONNECT_SUCCEED){
		//通知sdk 系统时间已就绪 可以进行闹钟初始化
		m_dcsSdk->notifySystemTimeReady();
		m_detectNTPTimer.stop();
		ActivityMonitorSingleton::getInstance()->updatePlayerStatus(PLAYER_STATUS_OFF);
		show_rt_time("exit detectNTPReady",inf);
		return ;
	}
	//获取到access token 30 秒后sdk 还没有授权成功,duer_linux 就自动退出（让monitor重启它）
	if((access("/tmp/aTokenInfo.txt",F_OK))!=-1) {
		if(detectNTPReadyCount++ > 30){
			SoundController::getInstance()->setRelinkSound(false);
			war("auth time out,duer_linux restart...");
			exit(-1);
		}
	}
	int ntp_update_state = cdb_get_int("$ntp_update_state",0);	
	for(;;){
		//ntpclient 获取到系统时间
		if(ntp_update_state == 1){
			break;
		}
		//系统没有连上路由器
		if(cdb_get_int("$wanif_state",0)!=2){
			return;
		}
		// 从 "baidu.com" 获取时间
		show_rt_time(__func__,war);
		platformNetworkTimeGet timeGet;
		timeGet.runNetworkTimeGet();
		if(timeGet.getTimeGetStateCode() == 0){
			break;
		}
		//获取网络时间失败 强行设置系统安全时间
		time_t oldTime = 0;
		int ret = time(&oldTime);
		#define SYSTEM_SAFE_TIME_STAMP 1547379885 // 2019/1/13 19:19:30
		if( ret < 0 || oldTime < SYSTEM_SAFE_TIME_STAMP){
			struct timespec ts = {SYSTEM_SAFE_TIME_STAMP,0};
			ret = clock_settime(CLOCK_REALTIME,&ts);
			if(ret < 0){
				err("set SYSTEM_SAFE_TIME_STAMP fialed!");
				break;
			}
			war("set SYSTEM_SAFE_TIME_STAMP done!");	
		}
		break;
	}
}

void DCSApplication::networkReady() {
    if (m_dcsSdk) {
        m_dcsSdk->notifyNetworkReady(DuerLinkWrapper::getInstance()->isFromConfigNetwork(), DeviceIoWrapper::getInstance()->getWifiBssid());
    }

#ifdef Build_CrabSdk
    /**
     * 网络连接成功，调用该接口上传上次崩溃转存的Crash文件
     */
    APP_INFO("crab upload crash dump file");
    baidu_crab_sdk::CrabSDK::upload_crash_async();
#endif
}

void DCSApplication::duerlinkNotifyReceivedData(const std::string& jsonPackageData, int sessionId) {
    if (!DeviceIoWrapper::getInstance()->isSleepMode()) {
        APP_INFO("DCSApplication duerlink_notify_received_data: [%s], sessionId: [%d]",
                 jsonPackageData.c_str(), sessionId);
        if (m_dcsSdk) {
            m_dcsSdk->consumeMessage(jsonPackageData);
        }
    }
}

void DCSApplication::duerlink_notify_received_bduss(const std::string& bdussValue) {
    if (m_dcsSdk) {
        m_dcsSdk->setBDUSS(bdussValue);
    }
}

void DCSApplication::setAccessToken(const std::string accessToken) {
    if (m_dcsSdk) {
        m_dcsSdk->setAccessToken(accessToken);
    }
}

void DCSApplication::duerlinkNetworkOnlineStatus(bool status) {
    if (m_dcsSdk) {
#ifdef Box86
        if (status) {
            if (!m_dcsSdk->isOAuthByPassPair()) {
                DuerLinkWrapper::getInstance()->waitLogin();
                DuerLinkWrapper::getInstance()->setFromConfigNetwork(true);
            } else {
                DuerLinkWrapper::getInstance()->setFromConfigNetwork(false);
            }
            m_dcsSdk->notifyNetworkReady(DuerLinkWrapper::getInstance()->isFromConfigNetwork(), DeviceIoWrapper::getInstance()->getWifiBssid());
        }
#else
        if (status && m_dcsSdk->isOAuthByPassPair()) {
		    m_dcsSdk->notifyNetworkReady(DuerLinkWrapper::getInstance()->isFromConfigNetwork(), DeviceIoWrapper::getInstance()->getWifiBssid());
        }
#endif
        m_dcsSdk->informOnlineStatus(status);
    }
}

void DCSApplication::recordDataInputCallback(const char* buffer, unsigned int size) {
    DeviceIoWrapper::getInstance()->transmitAudioToBluetooth(buffer, size);
    /// save record data to file.
    RecordAudioManager::getInstance()->transmitAudioRecordData(buffer, size);
}

}  // namespace application
}  // namespace duerOSDcsApp

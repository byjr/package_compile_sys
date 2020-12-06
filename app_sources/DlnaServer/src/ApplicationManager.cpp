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

#include "DCSApp/ApplicationManager.h"
#include "DCSApp/DuerLinkWrapper.h"
#include "DCSApp/DeviceIoWrapper.h"
#include "DCSApp/SoundController.h"
#include "DCSApp/Configuration.h"
#include "LoggerUtils/DcsSdkLogger.h"
#include <DeviceTools/StringUtils.h>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <rapidjson/istreamwrapper.h>
#include "DCSApp/AudioCallProxy.h"

#include <curl/curl.h>
extern "C"{
	extern unixFifoOps_t *cmccVoipFifoPtr;
	extern unixFifoOps_t *uartdFifoPtr;
}
namespace duerOSDcsApp {
namespace application {
using namespace duerOSDcsApp::voip;
static std::string QUERY_CURRENT_VERSION = "";
static const std::string POWER_SLEEP = "好的,已休眠";
static const std::string POWER_SHUTDOWN = "拔掉电源就可以关机了";
static const std::string POWER_REBOOT = "拔掉电源再插上就可以重启了";


static ssize_t write_callback(void* ptr, size_t size, size_t nmemb, autom_t *_autom) {
	ssize_t res = autom_write(_autom,(char *)ptr,nmemb*size);
	if(res < 0){
		return -1;
	}
	return nmemb;
}
static std::string curl_get(std::string &url,size_t size){
	autom_t *_autom = autom_create(NULL,1024);
	if(!_autom){
		err("autom_create out of memery!");
		return "";
	}
    CURL* curl_handle = NULL;
	FILE* fp =NULL;
	int oret = 0;
	std::string contex_string = "";
	for(;;){
	    curl_handle = curl_easy_init();
		if(!curl_handle){
			err("curl_easy_init failed!");
			oret = -1;
			break;
		}inf("curl_get:%s",url.c_str());
	    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
//	    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L); 
	    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
	    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &write_callback);
	    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, _autom);
		curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 15L);
	    CURLcode res = curl_easy_perform(curl_handle);
		if(res != CURLE_OK){
			err("curl_easy_perform failed:%d",res);
			oret = -2;
			break;
		}
		contex_string = _autom->head;
		break;	
	}
	switch(oret){
	case 0:
	case -2:curl_easy_cleanup(curl_handle);	
	case -1:autom_destroy(_autom);
		break;
	default:
		break;
	}
	return contex_string;
}




ApplicationManager *ApplicationManager::ApplicationManagerPtr=NULL;

ApplicationManager::ApplicationManager(){
	GetAccessTokenType = 1;
	ApplicationManagerPtr=this;
}

ApplicationManager *ApplicationManager::getInstance(){
	if(!ApplicationManagerPtr){
		err("ApplicationManager un create!");
	}
	return ApplicationManagerPtr;
}

void ApplicationManager::onDialogUXStateChanged(DialogUXState state) {
    if (state == m_dialogState) {
        return;
    }
    m_dialogState = state;
    onStateChanged();
    if (m_dcsSdk) {
        switch (state) {
            case DialogUXState::MEDIA_PLAYING:
#ifdef KITTAI_KEY_WORD_DETECTOR
                m_dcsSdk->enterPlayMusicScene();
#endif
                break;
            case DialogUXState::MEDIA_STOPPED:
            case DialogUXState::MEDIA_FINISHED:
                if (!(DeviceIoWrapper::getInstance()->isBtPlaying()
                || DeviceIoWrapper::getInstance()->isDlnaPlaying())) {
#ifdef KITTAI_KEY_WORD_DETECTOR
                    m_dcsSdk->exitPlayMusicScene();
#endif
                }
                break;
            default:
                break;
        }
    }
}

void ApplicationManager::onMessageSendComplete(bool success) {
    DeviceIoWrapper::getInstance()->setRecognizing(false);
    if (success) {
        APP_INFO("ApplicationManager::onMessageSendComplete(true)!");
    } else {
        APP_INFO("ApplicationManager::onMessageSendComplete(false)!");
    }
	m_isDialogValid=true;
    if (!m_isFromSpeaking) {
//        DeviceIoWrapper::getInstance()->ledSpeechOff();
    }
//	vfexec("recognizeResult.sh write",0);
	if(get_pids_by_name("ring.sh",NULL,1)){
		vfexec("ring.sh -r",0);
	}
}

void ApplicationManager::onReceivedDirective(const std::string &contextId, const std::string &message) {	  
 	if (message.empty()) {
        return;
    }
//	inf("ApplicationManager onReceivedDirective:%s", message.c_str());	
    std::string playbackctl_namespace = "ai.dueros.device_interface.speaker_controller";
    std::vector<std::string> filter_namespace;
    filter_namespace.emplace_back("ai.dueros.device_interface.voice_output");
    filter_namespace.emplace_back("ai.dueros.device_interface.audio_player");
    rapidjson::Document root;
    root.Parse<0>(message.c_str());
    if (!root.HasParseError()) {
        if (root.HasMember("directive") &&
            root["directive"].HasMember("header") &&
            root["directive"]["header"].HasMember("namespace")) {
            std::string headerNamespace = root["directive"]["header"]["namespace"].GetString();
            for (size_t i = 0; i < filter_namespace.size(); ++i) {
                if (filter_namespace[i].compare(headerNamespace) == 0) {
                    if (!m_isFirstReceiveMsg) {
                        APP_INFO("receive new audio directive, exit mute.");
                        DeviceIoWrapper::getInstance()->exitMute();
//                        DeviceIoWrapper::getInstance()->ledSpeechOff();
                        m_isFirstReceiveMsg = true;
                    }
                    break;
                }
            }
            if (headerNamespace == playbackctl_namespace) {
                if (!m_isFirstReceiveMsg) {
                    m_isFirstReceiveMsg = true;
                }
            }
        }
    }
}

void ApplicationManager::onAlertStateChange(const std::string &alertToken,
                                            DialogUXStateObserverInterface::AlertState state,
                                            const std::string &reason) {
    if (DialogUXStateObserverInterface::AlertState::STARTED == state) {
        APP_INFO("ApplicationManager onAlertStateChange STARTED");
        DeviceIoWrapper::getInstance()->setAlertRing(true);
//        DeviceIoWrapper::getInstance()->ledAlarm();
    } else if (DialogUXStateObserverInterface::AlertState::STOPPED == state
               || DialogUXStateObserverInterface::AlertState::COMPLETED == state) {
        APP_INFO("ApplicationManager onAlertStateChange STOPPED or COMPLETED");
        DeviceIoWrapper::getInstance()->setAlertRing(false);
//        DeviceIoWrapper::getInstance()->ledAlarmOff();
    }
}

#if defined(CONFIG_PANTHER)
void ApplicationManager::onListenStarted(bool success){
    APP_INFO(__func__);
	inf(__func__);
    if (m_micWrapper) {
		csrb_clear(m_micWrapper->m_csrb,'w');
        m_micWrapper->isCloudVadListening = true;
		m_micWrapper->isLocalVadListening = true;
		trd_timer_trigger(m_micWrapper->_trdTimer,NULL);
    }
	if(get_pids_by_name("ring.sh",NULL,1)){
		vfexec("ring.sh -p",0);
	}
	user_fifo_write_fmt(uartdFifoPtr,"tickCtrl 0");
}

void ApplicationManager::onStopListen(bool success) {
    APP_INFO(__func__);
	inf(__func__);
    if (m_micWrapper) {
		m_micWrapper->checkAndStopMutiVad();
    }
}
#endif

void ApplicationManager::onExpectSpeech(bool success) {	
    APP_INFO(__func__);
	inf(__func__);
    if (m_micWrapper) {
		m_micWrapper->excuteExpectSpeechResponse();
    }
//    DeviceIoWrapper::getInstance()->ledWakeUp(DeviceIoWrapper::getInstance()->getDirection());
}

void ApplicationManager::onSpeechFinished() {
    if (m_isFromSpeaking) {
//        DeviceIoWrapper::getInstance()->ledSpeechOff();
    }
}

void ApplicationManager::onConnectionStatusChanged(const Status status, const ChangedReason reason) {
    if (m_connectionStatus == status) {
        return;
    }
	if(status == Status::DISCONNECTED){
		err("sdk not connected,ChangedReason:%d",reason);
	}
    m_connectionStatus = status;
    onStateChanged();
}

void ApplicationManager::onSpeechAsrCanceled() {
//    DeviceIoWrapper::getInstance()->ledSpeechOff();
}

void ApplicationManager::setSpeakerVolume(int64_t volume) {
    APP_INFO("ApplicationManager setSpeakerVolume: %ld", volume);
    if (volume < 0) {
        APP_WARN("ApplicationManager setSpeakerVolume volume should not be negative value %ld", volume);
    }
	user_fifo_write_fmt(uartdFifoPtr,"setvol %03u",volume);
    DeviceIoWrapper::getInstance()->setCurrentVolume((unsigned int)volume);
    Configuration::getInstance()->setCommVol((unsigned int)volume);
}

void ApplicationManager::setSpeakerMute(bool isMute) {
    DeviceIoWrapper::getInstance()->setMute(isMute);
    if (isMute) {
        APP_INFO("ApplicationManager setSpeakerMute:Mute");
//        DeviceIoWrapper::getInstance()->ledMute();
    } else {
        APP_INFO("ApplicationManager setSpeakerMute:unMute");
//        DeviceIoWrapper::getInstance()->ledVolume();
    }
    DeviceIoWrapper::getInstance()->muteChanged();
}

int ApplicationManager::getSpeakerVolume() {
    return DeviceIoWrapper::getInstance()->getCurrentVolume();
}

bool ApplicationManager::getSpeakerMuteStatus() {
    return DeviceIoWrapper::getInstance()->isMute();
}

bool ApplicationManager::setStartDebugMode() {
    APP_INFO("ApplicationManager setStartDebug");
    LOGGER_ENABLE(true);
    // 当前版本暂时不禁止adb
//    if (Configuration::getInstance()->getDebug()) {
//        APP_INFO("ApplicationManager setStartDebug in debug mode");
//        system("systemctl start android-tools-adbd.service");
//    } else {
//        APP_INFO("ApplicationManager setStartDebug in release mode");
//        system("/data/usr/bin/android-gadget-setup adb && ( /data/usr/bin/adbd & )");
//    }

    debugStarted();
    return true;
}

bool ApplicationManager::setStopDebugMode() {
    APP_INFO("ApplicationManager setStopDebug");
    LOGGER_ENABLE(false);
    // 当前版本暂时不禁止adb
//    if (Configuration::getInstance()->getDebug()) {
//        system("systemctl stop android-tools-adbd.service");
//    } else {
//        system("killall -9 adbd");
//    }

    debugStoped();
    return true;
}

void ApplicationManager::debugStarted() {
    if (m_dcsSdk) {
        m_dcsSdk->debugStarted();
    }
}

void ApplicationManager::debugStoped() {
    if (m_dcsSdk) {
        m_dcsSdk->debugStoped();
    }
}

void ApplicationManager::setBluetoothStatus(bool status) {
    if (status) {
        APP_INFO("ApplicationManager setBluetoothStatus:Open");
        DeviceIoWrapper::getInstance()->openBluetooth();
		SoundController::getInstance()->openBluetooth(NULL,NULL);
    } else {
        APP_INFO("ApplicationManager setBluetoothStatus:Close");
        DeviceIoWrapper::getInstance()->closeBluetooth();
		SoundController::getInstance()->closeBluetooth(NULL,NULL);
    }
}

void ApplicationManager::setMicrophoneStatus(bool status) {
#if (defined Hodor) || (defined Kuke) || (defined Dot) || (defined Box86)
    if (!status) {
        APP_INFO("ApplicationManager setMicrophoneStatus: enterSleepMode");
        DeviceIoWrapper::getInstance()->enterSleepMode(true);
    }
#endif
}

void ApplicationManager::setBluetoothConnectionStatus(bool status) {
    if (status) {
        APP_INFO("UIManager setBluetoothConnectionStatus:true");
    } else {
        APP_INFO("UIManager setBluetoothConnectionStatus:false");
        if (DeviceIoWrapper::getInstance()->isBluetoothConnected()) {
            DeviceIoWrapper::getInstance()->unpairBluetooth();
        }
    }
}

void ApplicationManager::queryCurrentVersion() {
    inf("UIManager queryCurrentVersion");
	if(QUERY_CURRENT_VERSION.empty()){
		char *cmei = GetSnvalue();
		if(!cmei){
			SoundController::getInstance()->userTipSoundPlay("音箱CMEI号获取失败。",2,1);
			return;
		}
		char ver[16]="";
		cdb_get_str("$duerCmcc_ver",ver,sizeof(ver),NULL);
		std::stringstream streamVoice;
		streamVoice << "音箱CMEI号是:" << cmei << "/音箱版本号是:" << ver;
		free(cmei);
		QUERY_CURRENT_VERSION = streamVoice.str();			
	}
	SoundController::getInstance()->userTipSoundPlay(QUERY_CURRENT_VERSION,2,1);
}

void ApplicationManager::powerSleep() {
#if (defined Hodor) || (defined Kuke) || (defined Dot) || (defined Box86)
//    DeviceIoWrapper::getInstance()->ledTts();
    SoundController::getInstance()->playTts(POWER_SLEEP, true, ledTtsOffSleepCallback);
#endif
}

void ApplicationManager::powerShutdown() {
	user_fifo_write_fmt(uartdFifoPtr,"poweroff");
#ifdef LOCALTTS
#ifndef Sengled
//    DeviceIoWrapper::getInstance()->ledTts();
    SoundController::getInstance()->playTts(POWER_SHUTDOWN, true, ledTtsOffCallback);
#endif
#endif
}

void ApplicationManager::powerReboot() {
	vfexec("reboot",1);
#ifdef LOCALTTS
#ifndef Sengled
//    DeviceIoWrapper::getInstance()->ledTts();
//    SoundController::getInstance()->playTts(POWER_REBOOT, true, ledTtsOffCallback);
#endif
#endif
}

void ApplicationManager::informSdkConnectionStatus(duerOSDcsSDK::sdkInterfaces::SdkConnectionState sdkConnectionStatus) {
	cdb_set_int("$dcsSdk_connectStatus",(int)sdkConnectionStatus);
    switch (sdkConnectionStatus) {
    case duerOSDcsSDK::sdkInterfaces::SdkConnectionState::SDK_AUTH_FAILED:
        APP_INFO("ApplicationManager informSdkConnectionStatus: SDK_AUTH_FAILED");
#ifdef Box86
        if (m_dcsSdk && m_dcsSdk->isOAuthByPassPair()) {
            DeviceIoWrapper::getInstance()->ledNetLoginFailed();
            SoundController::getInstance()->accountUnbound(ApplicationManager::loginFailed);
        }
#else
//        DeviceIoWrapper::getInstance()->ledNetConnectFailed();
		if(m_dcsSdk && m_dcsSdk->isOAuthByPassPair()){
			SoundController::getInstance()->accountUnbound(nullptr);
		}
#endif
        break;

    case duerOSDcsSDK::sdkInterfaces::SdkConnectionState::SDK_AUTH_SUCCEED:
        APP_INFO("ApplicationManager informSdkConnectionStatus: SDK_AUTH_SUCCEED");
        break;

    case duerOSDcsSDK::sdkInterfaces::SdkConnectionState::SDK_CONNECT_FAILED:
        APP_INFO("ApplicationManager informSdkConnectionStatus: SDK_CONNECT_FAILED");
        break;

    case duerOSDcsSDK::sdkInterfaces::SdkConnectionState::SDK_CONNECT_SUCCEED:
        APP_INFO("ApplicationManager informSdkConnectionStatus: SDK_CONNECT_SUCCEED");
#ifdef CONFIG_PACKAGE_cmcc_voip
		if(get_pids_by_name("cmcc_voip",NULL,1) > 0){//如果cmcc_voip已经运行就发送下注册命令
			user_fifo_write_fmt(cmccVoipFifoPtr,"IMSevt reg");
		}else{//如果cmcc_voip没有运行就允许运行，等待monitor启动它
			cdb_set_int("$cmcc_voip_nocheck", 0);
		}
#endif
        if (DuerLinkWrapper::getInstance()->isFirstNetworkReady() || DuerLinkWrapper::getInstance()->isFromConfigNetwork()) {
            APP_INFO("ApplicationManager informSdkConnectionStatus: SDK_CONNECT_SUCCEED networkLinkOrRecoverySuccess");
            DuerLinkWrapper::getInstance()->networkLinkOrRecoverySuccess();
            /// because app is working no notify network link
            DuerLinkWrapper::getInstance()->setFirstNetworkReady(false);
            /// because app is working no notify network link, but after config network must notify
            DuerLinkWrapper::getInstance()->setFromConfigNetwork(false);
        }
        break;

    default:
        break;
    }
}

void ApplicationManager::setGetAccessTokenType(size_t type){
	GetAccessTokenType = type;
}
ssize_t path_fwrite(const void *buf,size_t bytes,size_t units,const char *path,char *mode){
	FILE* fp = fopen(path,mode);
	if(!fp){
		show_errno(0,"fopen");
		return -1;
	}
	if(!buf){
		fclose(fp);
		return 0;
	}
	size_t res = fwrite(buf,bytes,units,fp);
	if(res < 0){
		show_errno(0,"fwrite");
		fclose(fp);
		return -2;
	}
	fclose(fp);
	return units;
}
bool ApplicationManager::getAccessToken(std::string &accessToken) {
	std::lock_guard<std::mutex> lock{aTokenMtx};
	size_t localGetAccessTokenType = GetAccessTokenType;
	for(;!isDuerLinkNetworkReady() && localGetAccessTokenType != 2;){
		err("isDuerLinkNetworkReady is't ok,wait ...");
		return true;
	}
	std::string thirdUrl = "http://pingan.ccsmart.com.cn:8082/cmcc/";
	switch(localGetAccessTokenType){
	case 0:thirdUrl += "refreshAccessToken?client_id=" ;
		break;
	case 1:thirdUrl += "token?eventID=get&client_id=" ;
		break;
	case 2:thirdUrl += "clearAccessToken?client_id=" ;
		break;
	default:
		break;
	}
	thirdUrl += m_dcsSdk->getClientId();
	thirdUrl += "&device_id=";
	thirdUrl += DeviceIoWrapper::getInstance()->getDeviceId();
	for(;;){
		std::string curl_res = curl_get(thirdUrl,2048);
		if(curl_res.empty()){
			err("");
			break;
		}
		rapidjson::Document root;
		root.Parse<0>(curl_res.c_str());
		if(root.HasParseError()){
			err("");
			break;
		}
		if(!root.IsObject()){
			err("");
			break;
		}
		int result=root["ret"].GetInt();
		if(result){
			err("getAccessToken failed,err:%d",result);
			break;
		}
		if(2 == localGetAccessTokenType){//清除access token 成功
			inf("directive:%s",curl_res.c_str());
			accessToken = "";
			unlink("/tmp/aTokenInfo.txt");
			GetAccessTokenType = 1;
			return true;
		}
		accessToken = root["access_token"].GetString();
		if(accessToken.empty()){
			err("access_token is empty!");
			break;
		}
		inf("%s:%s",__func__,accessToken.c_str());
		path_fwrite(accessToken.c_str(),1,accessToken.length(),"/tmp/aTokenInfo.txt","w");
		GetAccessTokenType = 0;
		return true;
	}
    return false;
}
int parseJsonGetContactId(std::string &contex){
	rapidjson::Document ctxRoot;
	ctxRoot.Parse<0>(contex.c_str());
	if(ctxRoot.HasParseError()){
		err("HasParseError");
		return -1;
	}
	if(!ctxRoot.IsObject()){
		err("");
		return -1;;
	}
	if(!ctxRoot.HasMember("ret")){
		err("");
		return -1;
	}
	int result=ctxRoot["ret"].GetInt();
	if(result){
		err("getPhoneId failed,err:%d",result);
		return -1;
	}	
	if(!ctxRoot.HasMember("contactId")){
		return -1;
	}
	int contactId = ctxRoot["contactId"].GetInt();
	return contactId;
}

void ApplicationManager::informCustomizeDirective(const std::string &directive) {
	inf("ApplicationManager informCustomizeDirective: directive = [%s]", directive.c_str());
    APP_DEBUG("ApplicationManager informCustomizeDirective: directive = [%s]", directive.c_str());
	if (directive.empty()) {
		return;
	}
	rapidjson::Document root;
	root.Parse<0>(directive.c_str());
	if(root.HasParseError()){
		return ;
	}
	if(!root.IsObject()){
		return;
	}
	if( !root.HasMember("event") ||
		!root["event"].HasMember("header") ||
		!root["event"]["header"].HasMember("namespace")){
		return;
	}
	std::string headerNamespace = root["event"]["header"]["namespace"].GetString();
	if(!headerNamespace.compare("ai.dueros.device_interface.extensions.telephone")){
		if(!root["event"]["header"].HasMember("name")){
			return;
		}
		std::string headerName = root["event"]["header"]["name"].GetString();
		if(!headerName.compare("PhonecallByNumber")){
			if( !root["event"].HasMember("payload") ||
				!root["event"]["payload"].HasMember("callee") ||
				!root["event"]["payload"]["callee"].HasMember("phoneNumber")){
				return;
			}
			std::string payloadPhoneNumber = root["event"]["payload"]["callee"]["phoneNumber"].GetString();
			if(payloadPhoneNumber.empty()){
				return;
			}
			user_fifo_write_fmt(cmccVoipFifoPtr,"phoneEvent 0 %s",payloadPhoneNumber.c_str());
			inf("PhonecallByNumber:%s",payloadPhoneNumber.c_str());
		}else if(!headerName.compare("PhonecallByName")){
			if( !root["event"].HasMember("payload") ||
				!root["event"]["payload"].HasMember("candidateCallees")){
				return;
			}
			rapidjson::Value& ques = root["event"]["payload"]["candidateCallees"]; 
			if(!ques.IsArray()){
				err("");
				return ;
			}
			int i = 0;
			for(i=0;i<ques.Size();i++){
				rapidjson::Value& v = ques[i];
				if(!v.IsObject()){
					err("");
					continue;
				}
	            if (!v.HasMember("contactName") || !v["contactName"].IsString()) {  
					err("");
	                continue; 
	            } 								
				std::string contactName = v["contactName"].GetString();
				if(contactName.empty()){
					err("");
					continue;
				}
				char* escape_control = curl_escape(contactName.c_str(),contactName.size());
				contactName = escape_control;
				curl_free(escape_control);
				std::string url ="http://pingan.ccsmart.com.cn:8082/CMCC/getContactId?eventID=get&nickname=";				
				url += contactName;
				url += "&device_id=";
				url += DeviceIoWrapper::getInstance()->getDeviceId();
				std::string conetx = curl_get(url,2048);
				if(conetx.empty()){
					err("");
					continue;
				}
				int contactId=parseJsonGetContactId(conetx);
				if(contactId < 0){
					err("parseJsonGetContactId failed");
					continue;
				}
				user_fifo_write_fmt(cmccVoipFifoPtr,"phoneEvent 1 %d",contactId);
				inf("contactName:%d",contactName);
				break;				
			}		
		}else if(headerName.compare("SelectCallee")){
			inf("SelectCallee");
		}
	}else if(!headerNamespace.compare("ai.dueros.device_interface.thirdparty.cmcc.call")){
		if(!root["event"]["header"].HasMember("name")){
			return;
		}
		std::string headerName = root["event"]["header"]["name"].GetString();
		if(!headerName.compare("Qry_ver")){
			if(QUERY_CURRENT_VERSION.empty()){
				char *cmei = GetSnvalue();
				if(!cmei){
					SoundController::getInstance()->userTipSoundPlay("音箱CMEI号获取失败。",2,1);
					return;
				}
				char ver[16]="";
				cdb_get_str("$duerCmcc_ver",ver,sizeof(ver),NULL);
				std::stringstream streamVoice;
				streamVoice << "音箱CMEI号是:" << cmei << "/音箱版本号是:" << ver;
				free(cmei);
				QUERY_CURRENT_VERSION = streamVoice.str();			
			}
			SoundController::getInstance()->userTipSoundPlay(QUERY_CURRENT_VERSION,2,1);
		}else if(!headerName.compare("Qry_vol")){
			if(1 == cdb_get_int("$charge_plug",0)){
				SoundController::getInstance()->userTipSoundPlay("正在充电，请拔掉电源后重新查询！",2,1);
				return ;
			}
			char bat_vol[8] = {0};
			cdb_get("$battery_level",bat_vol);
			std::stringstream streamBat;
			streamBat << "音箱电量是：百分之" << (atoi(bat_vol)*10);
			SoundController::getInstance()->userTipSoundPlay(streamBat.str(),2,1);
		}else if(!headerName.compare("Receive")){
			user_fifo_write_fmt(cmccVoipFifoPtr,"phoneEvent 2");
			inf("Receive...");
		}else if(!headerName.compare("Hangup")){
			user_fifo_write_fmt(cmccVoipFifoPtr,"phoneEvent 3");
			inf("Hangup ...");
		}else if(!headerName.compare("Upload_log")){
			user_fifo_write_fmt(cmccVoipFifoPtr,"upLog");
			inf("Upload_log ...");
		}else if(!headerName.compare("Upgrade_ver")){
			inf("Upgrade_ver ...");
			vfexec("/usr/bin/ota",1);
		}
	}
}

bool ApplicationManager::linkThirdDevice(
        int deviceType, const std::string &clientId, const std::string& message) {
    return DuerLinkWrapper::getInstance()->startLoudspeakerCtrlDevicesService(
            static_cast<duerLink::device_type>(deviceType), clientId, message);
}

bool ApplicationManager::unlinkThirdDevice(int deviceType, const std::string& message) {
    return DuerLinkWrapper::getInstance()->disconnectDevicesConnectionsBySpeType(
            static_cast<duerLink::device_type>(deviceType), message);
}

bool ApplicationManager::onThirdDlpMessage(int deviceType, const std::string &message) {
    return DuerLinkWrapper::getInstance()->sendMsgToDevicesBySpecType(message,
                                                                      static_cast<duerLink::device_type>(deviceType));
}

void ApplicationManager::notifySdkContext(const std::string &context, int deviceType) {
	 DuerLinkWrapper::getInstance()->sendDlpMsgToAllClients(context);
	trc("%s:%s",__func__,context);
	if (context.empty()) {
		return;
	}
	rapidjson::Document root;
	root.Parse<0>(context.c_str());
	if(root.HasParseError()){
		return ;
	}
	if(!root.IsObject()){
		return;
	}
	if( !root.HasMember("to_client") ||
		!root["to_client"].HasMember("payload") ||
		!root["to_client"]["payload"].HasMember("type")){
		return ;
	}
	std::string type = root["to_client"]["payload"]["type"].GetString();
	if(!type.compare("FINAL")){
		if(!root["to_client"]["payload"].HasMember("text")){
			return ;
		}
		std::string text = root["to_client"]["payload"]["text"].GetString();
		inf("recognized text:%s",text.c_str());
	}
}

void ApplicationManager::notifySdkContextBySessionId(
        const std::string& context, unsigned short sessionId, int deviceType) {
    DuerLinkWrapper::getInstance()->sendDlpMessageBySessionId(context, sessionId);
}

bool ApplicationManager::systemInformationGetStatus(duerOSDcsSDK::sdkInterfaces::SystemInformation &systemInformation) {
    APP_INFO("ApplicationManager systemInformationGetStatus");
    systemInformation.ssid = DeviceIoWrapper::getInstance()->getWifiSsid();
    systemInformation.mac = DeviceIoWrapper::getInstance()->getWifiMac();
    systemInformation.ip = DeviceIoWrapper::getInstance()->getWifiIp();
    systemInformation.bluetoothMac = DeviceIoWrapper::getInstance()->getBtMac();
    systemInformation.deviceId = DeviceIoWrapper::getInstance()->getDeviceId();
    systemInformation.sn = DeviceIoWrapper::getInstance()->getDeviceId();
    systemInformation.softwareVersion = DeviceIoWrapper::getInstance()->getVersion();
    systemInformation.onlineStatus = DuerLinkWrapper::getInstance()->isNetworkOnline();
    systemInformation.deviceVersion = DeviceIoWrapper::getInstance()->getDeviceVersion();
    return true;
}

bool ApplicationManager::systemInformationHardReset() {
    APP_INFO("ApplicationManager systemInformationHardReset");
    DeviceIoWrapper::getInstance()->setTouchStartNetworkConfig(true);
#ifdef Box86
    DuerLinkWrapper::getInstance()->waitLogin();
    DuerLinkWrapper::getInstance()->setFromConfigNetwork(true);
#else
    DuerLinkWrapper::getInstance()->startNetworkConfig();
#endif
	
    return true;
}

bool ApplicationManager::systemUpdateGetStatus(duerOSDcsSDK::sdkInterfaces::SystemUpdateInformation &systemUpdateInformation) {
    APP_INFO("ApplicationManager systemUpdateGetStatus");
    system("upgrade_silent_client 2 GetStatus");

    return true;
}

bool ApplicationManager::systemUpdateUpdate() {
    APP_INFO("ApplicationManager systemUpdateUpdate");
    system("upgrade_silent_client 3 Update");

    return true;
}

bool ApplicationManager::getCurrentConnectedDeviceInfo(int deviceType,
                                   std::string& clientId, std::string& deviceId) {
    return DuerLinkWrapper::getInstance()->getCurrentConnectedDeviceInfo(
            static_cast<duerLink::device_type>(deviceType), clientId, deviceId);
}

std::string ApplicationManager::getWifiBssid() {
    return DeviceIoWrapper::getInstance()->getWifiBssid();
}

#ifdef SUPPORT_INFRARED
bool ApplicationManager::sendInfraredRayCodeRequest(int carrierFrequency, const std::string &pattern) {
    std::string carrierFrequencyStr = deviceCommonLib::deviceTools::convertToString(carrierFrequency);
    carrierFrequencyStr += ",";
    carrierFrequencyStr += pattern;
    APP_INFO("ApplicationManager send Infrared Ray Code Request: %s", carrierFrequencyStr.c_str());
    return 0 == DeviceIoWrapper::getInstance()->transmitInfrared(carrierFrequencyStr);
}
#endif

void ApplicationManager::setDcsSdk(std::shared_ptr<duerOSDcsSDK::sdkInterfaces::DcsSdk> dcsSdk) {
    m_dcsSdk = dcsSdk;
}

void ApplicationManager::setMicrophoneWrapper(std::shared_ptr<AudioMicrophoneInterface> micWrapper) {
    m_micWrapper = micWrapper;
}

void ApplicationManager::microphoneOn() {
    if (m_micWrapper) {
        m_micWrapper->startStreamingMicrophoneData();
		DeviceIoWrapper::getInstance()->led_open_MIC(NULL);
		cdb_set_int("$microphone_on",1);
		APP_INFO("ApplicationManager Microphone On!");
    }    
}

void ApplicationManager::microphoneOff() {
    if (m_micWrapper) {		
        m_micWrapper->stopStreamingMicrophoneData();
		DeviceIoWrapper::getInstance()->led_close_MIC(NULL);
		cdb_set_int("$microphone_on",0);
		APP_INFO("ApplicationManager Microphone Off!");
    }   
}

void ApplicationManager::stopMusicPlay() {
    if (m_dcsSdk) {
        m_dcsSdk->cancelMusicPlay();
        m_dcsSdk->cancelBluetoothPlay();
    }
}

void ApplicationManager::forceHoldFocus(bool holdFlag) {
    if (m_dcsSdk) {
        m_dcsSdk->forceHoldFocus(holdFlag);
    }
}

void ApplicationManager::cancelMusicPlay() {
    if (m_dcsSdk) {
        m_dcsSdk->cancelMusicPlay();
    }
}

void ApplicationManager::muteChanged(int volume, bool muted) {
    if (m_dcsSdk) {
        m_dcsSdk->muteChanged(volume, muted);
    }
}

void ApplicationManager::volumeChanged(int volume, bool muted) {
    if (m_dcsSdk) {
        m_dcsSdk->volumeChanged(volume, muted);
    }
}

void ApplicationManager::closeLocalActiveAlert() {
    if (m_dcsSdk) {
        m_dcsSdk->closeLocalActiveAlert();
    }
}

void ApplicationManager::ledTtsOffCallback() {
//    DeviceIoWrapper::getInstance()->ledSpeechOff();
}

void ApplicationManager::ledTtsOffSleepCallback() {
//    DeviceIoWrapper::getInstance()->ledSpeechOff();
    DeviceIoWrapper::getInstance()->enterSleepMode(true);
}

#define led_is_dialog_finished(isDialogValid) ({\
	if(isDialogValid){\
		DeviceIoWrapper::getInstance()->led_play_silence(NULL);\
		isDialogValid=false;\
	}\
})
void ApplicationManager::onStateChanged() {
    if (m_connectionStatus == duerOSDcsSDK::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED) {
        APP_INFO("ApplicationManager onStateChanged: Client not connected!");
    } else if (m_connectionStatus == duerOSDcsSDK::sdkInterfaces::ConnectionStatusObserverInterface::Status::PENDING) {
        APP_INFO("ApplicationManager onStateChanged: Connecting...");
    } else if (m_connectionStatus == duerOSDcsSDK::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED) {
        switch (m_dialogState) {
        case DialogUXState::IDLE:
//            DeviceIoWrapper::getInstance()->led_idel(NULL);
            APP_INFO("ApplicationManager onStateChanged: DuerOS is currently idle!");
            return;

        case DialogUXState::LISTENING:
			m_isDialogValid=false;
            m_isFromSpeaking = false;
            m_isFirstReceiveMsg = false;
            DeviceIoWrapper::getInstance()->led_listening(NULL);
            APP_INFO("ApplicationManager onStateChanged: Listening...");
            return;

        case DialogUXState::THINKING:
//          DeviceIoWrapper::getInstance()->ledAsr();
            DeviceIoWrapper::getInstance()->led_thinking(NULL);
            APP_INFO("ApplicationManager onStateChanged: Thinking...");
            return;;

        case DialogUXState::SPEAKING:
			m_micWrapper->checkAndStopMutiVad();
            m_isFromSpeaking = true;
//          DeviceIoWrapper::getInstance()->ledTts();
			DeviceIoWrapper::getInstance()->led_talking(NULL);
            APP_INFO("ApplicationManager onStateChanged: Speaking...");
            return;

        case DialogUXState::FINISHED:
			m_micWrapper->isSoundResponseDone = true;
            APP_INFO("ApplicationManager onStateChanged: SpeakFinished...");
       		led_is_dialog_finished(m_isDialogValid);
			DeviceIoWrapper::getInstance()->led_tts_finish(NULL);
            return;

        case DialogUXState::MEDIA_PLAYING:
            APP_INFO("ApplicationManager onStateChanged: Media Playing...");
			led_is_dialog_finished(m_isDialogValid);
            return;
        case DialogUXState::MEDIA_STOPPED:
            APP_INFO("ApplicationManager onStateChanged: Media Play Stopped...");
			led_is_dialog_finished(m_isDialogValid);
            return;

        case DialogUXState::MEDIA_FINISHED:
            APP_INFO("ApplicationManager onStateChanged: Media Play Finished...");
            return;
        }
    }
}



void ApplicationManager::loginFailed() {
//    DeviceIoWrapper::getInstance()->ledNetOff();
}

void ApplicationManager::playPauseButtonPressed(bool pausePressed) {
    if (pausePressed) {
        m_dcsSdk->issuePlaybackPauseCommand();
    } else {
        m_dcsSdk->issuePlaybackPlayCommand();
    }
}
void ApplicationManager::PlaybackNextPressed(){
    m_dcsSdk->issuePlaybackNextCommand();
}

void ApplicationManager::PlaybackPrevious(){
	m_dcsSdk->issuePlaybackPreviousCommand();
}
}  // namespace application
}  // namespace duerOSDcsApp

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


#include "DCSApp/Audio/AlsaAudioMicrophoneWrapper.h"
#include "DCSApp/DeviceIoWrapper.h"
#include "DCSApp/RecordAudioManager.h"
#include "LoggerUtils/DcsSdkLogger.h"
#include "DCSApp/SoundController.h"
#include "DCSApp/DuerLinkWrapper.h"
#include "DcsSdk/ApplicationImplementation.h"
#include "DCSApp/DuerLinkWrapper.h"
#include "DCSApp/Configuration.h"

//#if defined(CONFIG_PANTHER)
#include <snowboy-detect.h>
//#endif
#define RESOURE_FILE_NAME   "./resources/universal_detect.res"
#define RESOURE_MODEL_NAME  "./resources/hotword.umdl"
#define HIGHT_SENSITIVITY   "0.52"
#define SENSITIVITY         "0.45"
#define SNOWBOY_GAIN        3.8f

#define VEPINPUT_PERIOD_GET_BYTES 1280 //40ms

extern "C"{
int record_test(size_t nSec){
	vepinput_t* _vepinput=vepinput_create();
	if(!_vepinput){
		err("");
		return -1;
	}
	vepinput_resume(_vepinput);
	war("record_test %d seconds ...",nSec);
	if(nSec){
		vepinput_bfp(_vepinput);//启动录音数据保存
		sleep(nSec);
		war("record_test done!");
		exit(0);
	}
	while(1)pause();
}

int brokenWakeup_test(){
	size_t wakeupTimesCount = 0;
	std::ofstream wakeupTimes;
	wakeupTimes.open("/tmp/wakeupTimes");
	vepinput_t* _vepinput=vepinput_create();
	if(!_vepinput){
		err("");
		return -1;
	}
	char buf[16]="";
	war("brokenWakeup_test ...");
  	SnowboyInit(RESOURE_FILE_NAME, RESOURE_MODEL_NAME, 1);
	if(cdb_get_str("$snowBoyHSens",buf,sizeof(buf),NULL)){
		war("snowBoyHSens:%s",buf);
		SnowboySetHighSensitivity(buf);
	}else{
		//SnowboySetHighSensitivity(HIGHT_SENSITIVITY);
	}
	if(cdb_get_str("$snowBoyGain",buf,sizeof(buf),NULL)){
		war("snowBoyGain:%f",atof(buf));
		SnowboySetAudioGain((const float )atof(buf));
	}else{
		//SnowboySetAudioGain(SNOWBOY_GAIN);
	}
	bzero(buf,sizeof(buf));
	if(cdb_get_str("$snowBoySens",buf,sizeof(buf),NULL)){
		war("snowBoySens:%s",buf);
		SnowboySetSensitivity(buf);
	}else{
		SnowboySetSensitivity(SENSITIVITY);
	}
	struct timespec tv={0,1000*1000*20};
	trc_time_t *_trcTime = trc_time_create(CLOCK_MONOTONIC,&tv);
	if(!_trcTime){
		err("trc_time_create failed!");
		return NULL;
	}
	if(posix_thread_set_nice(-20)){
		err("set nice failed!");
	};
	vepinput_resume(_vepinput);
	while(1){
		trc_time_proc_delay(_trcTime);
//		show_rt_time("",inf);
		trc_time_proc_sync(_trcTime);
		char *vep_ri = csrb_read_query(_vepinput->_csrb,VEPINPUT_PERIOD_GET_BYTES);
		if(!vep_ri) continue;
		int rret = SnowboyRunDetection((const int16_t *)vep_ri,VEPINPUT_PERIOD_GET_BYTES/2,0);
		if(rret > 0){
			system("killall aplay 2>/dev/null;aplay -Ddefault -fs16_le -c1 -r16000 /root/appresources/du.pcm &");
			wakeupTimes << "===wakeupTimes:<" << ++wakeupTimesCount << ">=============" << std::endl;
			war("\n\n===wakeupTimes:<%u>==========\n",wakeupTimesCount);
			SnowboyReset();
		}
		csrb_read_sync(_vepinput->_csrb,VEPINPUT_PERIOD_GET_BYTES);
	}
}
}


// the class factories
extern "C" std::shared_ptr<duerOSDcsApp::application::AudioMicrophoneInterface> create(std::shared_ptr<duerOSDcsSDK::sdkInterfaces::DcsSdk> dcsSdk) {
    return duerOSDcsApp::application::AlsaAudioMicroponeWrapper::create(dcsSdk);
}
using namespace duerOSDcsSDK::sdkInterfaces;

namespace duerOSDcsApp {
namespace application {

std::unique_ptr<AlsaAudioMicroponeWrapper> AlsaAudioMicroponeWrapper::create(
        std::shared_ptr<duerOSDcsSDK::sdkInterfaces::DcsSdk> dcsSdk) {
    if (!dcsSdk) {
        APP_ERROR("Invalid dcsSdk passed to AlsaAudioMicrophoneWrapper");
        return nullptr;
    }
    std::unique_ptr<AlsaAudioMicroponeWrapper> alsaAudioMicrophoneWrapper(new AlsaAudioMicroponeWrapper(dcsSdk));
    if (!alsaAudioMicrophoneWrapper->initialize()) {
        APP_ERROR("Failed to initialize AlsaAudioMicrophoneWrapper");
        return nullptr;
    }
    return alsaAudioMicrophoneWrapper;
}

AlsaAudioMicroponeWrapper::AlsaAudioMicroponeWrapper(std::shared_ptr<duerOSDcsSDK::sdkInterfaces::DcsSdk> dcsSdk) :
        m_dcsSdk{dcsSdk},
//        _m_rec{nullptr},
        m_callBack{nullptr} {
		running=false;
}

AlsaAudioMicroponeWrapper::~AlsaAudioMicroponeWrapper() {
//    _m_rec->recorderClose();
}
void AlsaAudioMicroponeWrapper::cmccVoipRcordingstarted(void) {
	inf(__func__);
	vepinput_pause(_vepinput);
	isCmccVoipRcording = true;
}
void AlsaAudioMicroponeWrapper::cmccVoipRcordingStoped(void) {
	inf(__func__);
	vepinput_resume(_vepinput);
	isCmccVoipRcording = false;
}
#define silenceSwitchToWifiMode() ({\
       if(cdb_get_int("$playmode",0) == 1){\
               user_fifo_write_fmt(uartdFifoPtr,"errwak");\
               sleep(1);\
       }\
})
void AlsaAudioMicroponeWrapper::excuteWakeupCheck(void) {
    if ((DUERLINK_NETWORK_SUCCEEDED == DuerLinkWrapper::getInstance()->getNetworkStatus()) ||
        (DUERLINK_NETWORK_CONFIG_SUCCEEDED == DuerLinkWrapper::getInstance()->getNetworkStatus()) ||
        (DUERLINK_NETWORK_RECOVERY_SUCCEEDED == DuerLinkWrapper::getInstance()->getNetworkStatus())) {
		if (!m_dcsSdk) {
			return;
		}
		SdkConnectionState m_sdkConnectionStates = m_dcsSdk->getSdkConnectionStates();
		APP_INFO("VoiceAndTouchWakeUpObserver wakeupTriggered:%d", m_sdkConnectionStates);
		if (m_sdkConnectionStates == SdkConnectionState::SDK_CONNECT_SUCCEED) {
			SoundController::getInstance()->wakeUp();
			m_dcsSdk->notifyOfTapToTalk();
		} else {
			silenceSwitchToWifiMode();
			if (m_sdkConnectionStates == SdkConnectionState::SDK_AUTH_FAILED) {
				err("%s:%s",__func__,"SDK_AUTH_FAILED");
				DeviceIoWrapper::getInstance()->setRecognizing(false);
				SoundController::getInstance()->accountUnbound(nullptr);
			} else if (m_sdkConnectionStates == SdkConnectionState::SDK_CONNECT_FAILED) {
				err("%s:%s",__func__,"SDK_CONNECT_FAILED");
				DeviceIoWrapper::getInstance()->setRecognizing(false);
				SoundController::getInstance()->serverConnectFailed();
			} else if (m_sdkConnectionStates == SdkConnectionState::SDK_CONNECTING) {
				err("%s:%s",__func__,"SDK_CONNECTING");
				DeviceIoWrapper::getInstance()->setRecognizing(false);
				SoundController::getInstance()->serverConnecting();
			}
		}
		DuerLinkWrapper::getInstance()->verifyNetworkStatus(true);
	} else {
		silenceSwitchToWifiMode();
		DeviceIoWrapper::getInstance()->setRecognizing(false);
		if (DeviceIoWrapper::getInstance()->isAlertRing()) {
			err("%s:%s",__func__,"isAlertRing");
			DeviceIoWrapper::getInstance()->closeLocalActiveAlert();
		} else {
			err("%s:%s",__func__,"notAlertRing");
			DeviceIoWrapper::getInstance()->exitMute();
			DuerLinkWrapper::getInstance()->wakeupDuerLinkNetworkStatus();
		}
    }
}
void AlsaAudioMicroponeWrapper::excuteWakeupResponse(void) {
	isSoundResponseDone = false;
	checkAndStopMutiVad();
	excuteWakeupCheck();
	vad_args.savedStatus=VAD_CHECK_IDEL;
	wakeupTimes.seekp(0);
	wakeupTimes << "===wakeupTimes:<" << ++wakeupTimesCount << ">=============" << std::endl;
	war("\n\n===wakeupTimes:<%u>==========\n",wakeupTimesCount);
#ifdef BACKUP_THE_UPLOADED_DATE_PATH
	backupFile.clear();
	backupFile.close();
	backupFile.open(BACKUP_THE_UPLOADED_DATE_PATH);
#endif
	throwInvaildCount = 0;
	throwInvaildMax = 15;
}
void AlsaAudioMicroponeWrapper::excuteExpectSpeechResponse(void) {
#ifdef BACKUP_THE_UPLOADED_DATE_PATH
	backupFile.clear();
	backupFile.close();
	backupFile.open(BACKUP_THE_UPLOADED_DATE_PATH);
#endif
	vad_args.savedStatus=VAD_CHECK_IDEL;
	throwInvaildCount = 0;
	throwInvaildMax = 15;
	checkAndStopMutiVad();
}
void AlsaAudioMicroponeWrapper::checkAndStopMutiVad(){
	if(isCloudVadListening || isLocalVadListening){
		isCloudVadListening = false;
		isLocalVadListening = false;
		trd_timer_stop(_trdTimer);
		trc("%s:MutiVad stop...!",__func__);
	}else{
		trc("%s:MutiVad stoped!",__func__);
	}
}

void AlsaAudioMicroponeWrapper::recordDataInputCallback(char* buffer, long unsigned int size, void *userdata) {
	auto m_micWrapper = static_cast<AlsaAudioMicroponeWrapper*>(userdata);
    if (m_micWrapper->running == false) {
        return ;
    }
	int rret = SnowboyRunDetection((const int16_t *)buffer,size/2,0);
	if(rret > 0){
		m_micWrapper->excuteWakeupResponse();
		SnowboyReset();
	}
	if(!m_micWrapper->isSoundResponseDone) {
		return ;
	}
    if(!m_micWrapper->isCloudVadListening){
		return;
	}
	if( ++m_micWrapper->throwInvaildCount < m_micWrapper->throwInvaildMax){
		return ;
	}
	if(m_micWrapper->vad_args.vadThreshold){
		if(!m_micWrapper->isLocalVadListening){
			return ;
		}
		m_micWrapper->vad_args.sample=(short *)buffer;
		m_micWrapper->vad_args.bytes=size;
		int vad_status=get_vad_status(&m_micWrapper->vad_args);
	//	inf("vad_status:%d",vad_status);
		switch(vad_status){
		case VAD_CHECK_IDEL:
		case VAD_CHECK_STARTED:
		case VAD_CHECK_SPEAKING:
			break;
		case VAD_CHECK_FINISHED:
			inf("notifyOfTapToTalkEnd");
			m_micWrapper->m_dcsSdk->notifyOfTapToTalkEnd();
			m_micWrapper->vad_args.savedStatus=VAD_CHECK_DONE;
			m_micWrapper->checkAndStopMutiVad();
			return;
		case VAD_CHECK_DONE:
			return;
		default:
			return;
		}
	}
	char *wi = csrb_write_query(m_micWrapper->m_csrb,size);
	if(!wi){
		err("upload csrb is full");
		return ;
	}
	memcpy(wi,buffer,size);
	csrb_write_sync(m_micWrapper->m_csrb,size);
#ifdef BACKUP_THE_UPLOADED_DATE_PATH
	m_micWrapper->backupFile.write(buffer,size);
#endif
}
void * AlsaAudioMicroponeWrapper::uploadThreadRoutine(void *args){
	AlsaAudioMicroponeWrapper* m_micWrapper = (AlsaAudioMicroponeWrapper *)args;
	struct timespec tv={0,1000*1000*20};
	trc_time_t *_trcTime = trc_time_create(CLOCK_MONOTONIC,&tv);
	if(!_trcTime){
		err("trc_time_create failed!");
		return NULL;
	}
	for(;;){
		trc_time_proc_delay(_trcTime);
		trc_time_proc_sync(_trcTime);
		char *ri = csrb_read_query(m_micWrapper->m_csrb,VEPINPUT_PERIOD_GET_BYTES);
		if(!ri){
			continue;
		}
		trc("uploading ... !");
	    ssize_t returnCode = m_micWrapper->m_dcsSdk->writeAudioData(ri, VEPINPUT_PERIOD_GET_BYTES/2);
	    if (returnCode <= 0) {
			err("writeAudioData faild,returnCode=%d",returnCode);
	        APP_ERROR("Failed to write to stream.");
	    }
		csrb_read_sync(m_micWrapper->m_csrb,VEPINPUT_PERIOD_GET_BYTES);
	}
}

void * AlsaAudioMicroponeWrapper::wakeupThreadRoutine(void *args){
	AlsaAudioMicroponeWrapper* m_micWrapper = (AlsaAudioMicroponeWrapper *)args;
	struct timespec tv={0,1000*1000*20};
	trc_time_t *_trcTime = trc_time_create(CLOCK_MONOTONIC,&tv);
	if(!_trcTime){
		err("trc_time_create failed!");
		return NULL;
	}
	if(posix_thread_set_nice(-20)){
		err("set nice failed!");
	};
	vepinput_resume(m_micWrapper->_vepinput);
	for(;;){
		trc_time_proc_delay(_trcTime);
		trc_time_proc_sync(_trcTime);
		if(m_micWrapper->isCmccVoipRcording){
			char *raddr = shrb_read_query(m_micWrapper->_shrb,VEPINPUT_PERIOD_GET_BYTES);
			if(!raddr){continue;}
			recordDataInputCallback(raddr,VEPINPUT_PERIOD_GET_BYTES,m_micWrapper);
			shrb_read_sync(m_micWrapper->_shrb,VEPINPUT_PERIOD_GET_BYTES);
		}else{
			char *vep_ri = csrb_read_query(m_micWrapper->_vepinput->_csrb,VEPINPUT_PERIOD_GET_BYTES);
			if(!vep_ri){continue;}
			recordDataInputCallback(vep_ri,VEPINPUT_PERIOD_GET_BYTES,m_micWrapper);
			csrb_read_sync(m_micWrapper->_vepinput->_csrb,VEPINPUT_PERIOD_GET_BYTES);
		}
	}
}
static void trdTimerCallback(union sigval val){
	AlsaAudioMicroponeWrapper *m_micWrapper = (AlsaAudioMicroponeWrapper*)val.sival_ptr;
	war(__func__);
	if(m_micWrapper->isCloudVadListening || m_micWrapper->isLocalVadListening){
		m_micWrapper->isCloudVadListening = false;
		m_micWrapper->isLocalVadListening = false;
		trd_timer_stop(m_micWrapper->_trdTimer);
	}
}
bool AlsaAudioMicroponeWrapper::initialize() {
    APP_INFO("use alsa recorder");
#ifdef BACKUP_THE_UPLOADED_DATE_PATH
	backupFile.open(BACKUP_THE_UPLOADED_DATE_PATH);
#endif
	wakeupTimes.open("/tmp/wakeupTimes");
	wakeupTimes<<"0"<<std::endl;
//	vad_args.vadThreshold=100*10000;
	vad_args.vadThreshold = cdb_get_int("$localvad_trd",0);
	if(vad_args.vadThreshold){
		vad_args.startedConfirmCount=8;
		vad_args.finishedConfirmCount=12;
		vad_args.unitBytes=320;	
	}
	running = true;
	isCloudVadListening = false;
	isCmccVoipRcording = false;
	isLocalVadListening = false;
	isSoundResponseDone = false;

#define VCPDATA_PATH "vcpdata"
#define VCPDATA_BUFF_SIZE (1280*2)
	_shrb = shrb_get(VCPDATA_PATH,VCPDATA_BUFF_SIZE);
	if(!_shrb){
		err("shrb_get");
		return false;
	}

	_vepinput = vepinput_create();
	if(!_vepinput){
		err("vepinput_create");
		return false;
	}

	m_csrb = csrb_create(NULL,VEPINPUT_PERIOD_GET_BYTES*2);
	if(!m_csrb){
		err("csrb_create");
		return false;
	}

	trd_timer_args_t args = {0};
	struct itimerspec it = {{10,0},{12,0}};
	memcpy(&args.its,&it,sizeof(args.its));
	args.handle = &trdTimerCallback;
	args.arg.sival_ptr=this;
	_trdTimer = trd_timer_create(&args);
	if(!_trdTimer){
		err("trd_timer_create");
		return false;
	}

	SnowboyInit(RESOURE_FILE_NAME, RESOURE_MODEL_NAME, 1);
//	SnowboySetHighSensitivity(HIGHT_SENSITIVITY);
	SnowboySetSensitivity(SENSITIVITY);
//	SnowboySetAudioGain(SNOWBOY_GAIN);
    pthread_create(&wakeupThreadId,NULL, wakeupThreadRoutine, this);
    pthread_setname_np(wakeupThreadId, "wakeupThread");
    pthread_create(&uploadThreadId,NULL, uploadThreadRoutine, this);
    pthread_setname_np(uploadThreadId, "uploadThread");
	return true;
}

bool AlsaAudioMicroponeWrapper::startStreamingMicrophoneData() {
    std::lock_guard<std::mutex> lock{m_mutex};
    running = true;
	vepinput_resume(_vepinput);
    return true;
}

void AlsaAudioMicroponeWrapper::setRecordDataInputCallback(void (*callBack)(const char *buffer, unsigned int size)) {
    m_callBack = callBack;
}

bool AlsaAudioMicroponeWrapper::stopStreamingMicrophoneData() {
    std::lock_guard<std::mutex> lock{m_mutex};
    running = false;
	vepinput_pause(_vepinput);
    return true;
}

}  // namespace application
}  // namespace duerOSDcsApp

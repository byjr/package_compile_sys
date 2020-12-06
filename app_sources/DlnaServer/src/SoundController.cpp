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

#include <DCSApp/Configuration.h>
#include "DCSApp/SoundController.h"
#include "DeviceTools/TimeUtils.h"
#include "LoggerUtils/DcsSdkLogger.h"
#include "DCSApp/ActivityMonitorSingleton.h"
#include "DCSApp/DeviceIoWrapper.h"

namespace duerOSDcsApp {
namespace application {

#define MIN_PLAY_TIMEINTERVAL 300

using namespace duerOSDcsSDK::sdkInterfaces;
using namespace deviceCommonLib::deviceTools;

SoundController *SoundController::mSoundController = NULL;
pthread_once_t SoundController::m_initOnce = PTHREAD_ONCE_INIT;
pthread_once_t SoundController::m_destroyOnce = PTHREAD_ONCE_INIT;
mediaPlayer::PcmFilePlayerImpl *SoundController::m_pcmPlayer=NULL;

SoundController::SoundController() {
    pthread_mutex_init(&m_mutex, NULL);
    m_lastPlayTimestamp = 0L;
    m_destroyOnce = PTHREAD_ONCE_INIT;
    m_pcmPlayer = new mediaPlayer::PcmFilePlayerImpl();
//	if(cdb_get_int("$duer_reset",0)){
//		relink_sound = false;
//		return;
//	}
//	relink_sound = true;
}

SoundController::~SoundController() {
    pthread_mutex_destroy(&m_mutex);
    m_initOnce = PTHREAD_ONCE_INIT;
}

SoundController *SoundController::getInstance() {
    pthread_once(&m_initOnce, &SoundController::init);
    return mSoundController;
}

void SoundController::addObserver(std::shared_ptr<LocalSourcePlayerInterface> mediaInterface) {
    if (mediaInterface) {
        m_observers.insert(mediaInterface);
    }
}

void SoundController::init() {
    if (mSoundController == NULL) {
        mSoundController = new SoundController();
    }
}

void SoundController::destory() {
    if (mSoundController) {
        delete mSoundController;
        mSoundController = NULL;
    }
}

void SoundController::release() {
    pthread_once(&m_destroyOnce, SoundController::destory);
}

void SoundController::audioPlay(const std::string &source,
                                bool needFocus,
                                void (*start_callback)(void *arg),
                                void *start_cb_arg,
                                void (*finish_callback)()) {
#if defined(CONFIG_PANTHER)
    unsigned long currentTimestamp = currentTimeMs();
#else
    uint64_t currentTimestamp = currentTimeMs();
#endif
    if (currentTimestamp - m_lastPlayTimestamp < MIN_PLAY_TIMEINTERVAL) {
        APP_INFO("frequent calls, return");
        return;
    }
    m_lastPlayTimestamp = currentTimestamp;
    for (auto observer : m_observers) {
        if (observer) {
            observer->playLocalSource(source, needFocus, start_callback, start_cb_arg, finish_callback);
        }
    }
}
void SoundController::forceHoldFocusEnter(void *args){
	auto managerPtr=ApplicationManager::getInstance();
	if(managerPtr){
		 managerPtr->forceHoldFocus(true);
	}	
}
void SoundController::forceHoldFocusExit(){
	auto managerPtr=ApplicationManager::getInstance();
	if(managerPtr){
		 managerPtr->forceHoldFocus(false);
	}	
}
static void soudWakeupFinished(){
	auto managerPtr=ApplicationManager::getInstance();
	if(managerPtr && managerPtr->m_micWrapper){
		managerPtr->m_micWrapper->isSoundResponseDone = true;
	}
}

void SoundController::wakeUp() {
	if(cdb_get_int("$playmode",0) == 1){//如果当前处于蓝牙模式
	 	user_fifo_write_fmt(uartdFifoPtr,"wakeup");
		DeviceIoWrapper::getInstance()->led_wakeup(NULL);
		//cslog("bt wakeup");
		usleep(200*1000);
		soudWakeupFinished();
		inf("force switch to wifi mode !");
	}
	else
	{
		DeviceIoWrapper::getInstance()->led_wakeup(NULL);
		//cslog("wifi wakeup");
		m_pcmPlayer->play("./appresources/du.pcm",NULL, NULL,soudWakeupFinished);
	}
}

void SoundController::commonStartCB(void *arg){
	user_fifo_write_fmt(uartdFifoPtr,"duerLedSet 101");
//	user_fifo_write_fmt(uartdFifoPtr,"duerLedSet %03u",(unsigned int)duerScene_t::TIP_SOUND_START);
}

void SoundController::commonFinishCB(void){
	user_fifo_write_fmt(uartdFifoPtr,"duerLedSet 102");
//	user_fifo_write_fmt(uartdFifoPtr,"duerLedSet %03u",(unsigned int)duerScene_t::TIP_SOUND_END);
}
void SoundController::linkStartedCallback(void *args){
	DeviceIoWrapper::getInstance()->led_connect_succeed(NULL);
//	sensoryRecgnizeClose(NULL);
}

void SoundController::linkFinshedCallback(){
	commonFinishCB();
//	sensoryRecgnizeOpen();
//	inf(__func__);
	user_fifo_write_fmt(uartdFifoPtr,"succeed");
}

inline static void  linkStartFirstStartedCallback(void *arg){
	 DeviceIoWrapper::getInstance()->led_first_poweron(arg);
//	sensoryRecgnizeClose(NULL);
}
void SoundController::AudioCallErrTip() {
    audioPlay("./cmcc_tipsound/cmccVoip_busy.mp3", true,
		DeviceIoWrapper::getInstance()->led_talking, NULL, commonFinishCB);
}

void SoundController::linkStartFirst() {
    audioPlay("./appresources/link_start_first.mp3", true,
		linkStartFirstStartedCallback, NULL, commonFinishCB);
}

inline static void  linkStartstartedCallback(void *arg){
	 DeviceIoWrapper::getInstance()->led_connect_start(arg);
}

void SoundController::linkStart() {
    audioPlay("./appresources/link_start.mp3", true, 
		linkStartstartedCallback, NULL, NULL);
}

void SoundController::linkConnecting() {
    audioPlay("./appresources/link_connecting.mp3", true, 
		DeviceIoWrapper::getInstance()->led_connecting, NULL, NULL);
}

void SoundController::linkSuccess(void(*callback)()) {
    audioPlay("./appresources/link_success.mp3", true,
		linkStartedCallback, NULL, linkFinshedCallback);
}

void SoundController::linkFailedPing(void(*callback)()) {
    audioPlay("./appresources/link_failed_ping.mp3", true,
 		 DeviceIoWrapper::getInstance()->led_network_fault, NULL, commonFinishCB);
}

void SoundController::linkFailedIp(void(*callback)()) {
    audioPlay("./appresources/link_failed_ip.mp3", true,
		 DeviceIoWrapper::getInstance()->led_connection_routing_failed, NULL, commonFinishCB);
}
void SoundController::linkExitFinishedCallback(){

		user_fifo_write_fmt(uartdFifoPtr,"failed");
}
void SoundController::linkExit(void(*callback)()) {
    audioPlay("./appresources/link_exit.mp3", true,
	 	DeviceIoWrapper::getInstance()->led_connect_cancle, NULL, linkExitFinishedCallback);
}

void SoundController::reLink() {
	if(relink_sound){
	    audioPlay("./appresources/re_link.mp3", true,
			DeviceIoWrapper::getInstance()->led_device_boot, NULL, NULL);
	}
}

void SoundController::reLinkSuccess(void(*callback)()) {
	if(relink_sound){
		audioPlay("./appresources/re_link_success.mp3", true,
			linkStartedCallback, NULL, linkFinshedCallback);
	}
}

void SoundController::reLinkFailed() {
    audioPlay("./appresources/re_link_failed.mp3", true, 
		DeviceIoWrapper::getInstance()->led_network_fault, NULL, commonFinishCB);
}

void SoundController::btUnpaired() {
    audioPlay("./appresources/bt_unpaired.mp3", true, NULL, NULL, NULL);
}

void SoundController::btPairSuccess(void(*callback)()) {
    audioPlay("./appresources/bt_pair_success.mp3", true, NULL, NULL, callback);
}

void SoundController::btPairFailedPaired() {
    audioPlay("./appresources/bt_pair_failed_paired.mp3", true, NULL, NULL, NULL);
}

void SoundController::btPairFailedOther() {
    audioPlay("./appresources/bt_pair_failed_other.mp3", true, NULL, NULL, NULL);
}

void SoundController::btDisconnect(void(*callback)()) {
    audioPlay("./appresources/bt_disconnect.mp3", true, NULL, NULL, callback);
}

void SoundController::networkConnectFailed() {
    audioPlay("./appresources/network_connect_failed.mp3", true,
		//DeviceIoWrapper::getInstance()->led_network_exception, NULL, commonFinishCB);
		DeviceIoWrapper::getInstance()->led_network_exception, NULL, NULL);
}

void SoundController::networkSlow() {
    audioPlay("./appresources/network_slow.mp3", true,
		 DeviceIoWrapper::getInstance()->led_slow_network, NULL, NULL);
}

void SoundController::openBluetooth(void(*callback)(void *arg), void *arg) {
    audioPlay("./appresources/open_bluetooth.mp3", true, callback, arg, NULL);
}

void SoundController::closeBluetooth(void(*callback)(void *arg), void *arg) {
    audioPlay("./appresources/close_bluetooth.mp3", true, callback, arg, NULL);
}

void SoundController::volume() {
//    m_pcmPlayer->setPlayType(mediaPlayer::PLAYTYPE_KEYCLICK);
//    m_pcmPlayer->stop();
//    m_pcmPlayer->play();
    ActivityMonitorSingleton::getInstance()->updatePlayerStatus(PLAYER_STATUS_ON);
}

void SoundController::serverConnecting() {
    audioPlay("./appresources/server_connecting.mp3", false, NULL, NULL, NULL);
}

void SoundController::serverConnectFailed() {
    audioPlay("./appresources/server_connect_failed.mp3", false,
		DeviceIoWrapper::getInstance()->led_network_exception, NULL, NULL);
}

void SoundController::bleNetworkConfig() {
    audioPlay("./appresources/ble_network_config.mp3", false, NULL, NULL, NULL);
}

void SoundController::accountUnbound(void(*callback)()) {
    audioPlay("./appresources/unbound.mp3", true,
		DeviceIoWrapper::getInstance()->led_unbind_success,NULL,commonFinishCB);
}

void SoundController::hotConnected() {
    audioPlay("./appresources/hot_connected.mp3", false, NULL, NULL, NULL);
}

void SoundController::microphoneOn(){
	audioPlay("./appresources/mic_on.mp3", true, NULL, NULL, NULL);
}
void SoundController::microphoneOff(){
	audioPlay("./appresources/mic_off.mp3", true, NULL, NULL, NULL);
}

void SoundController::waitLogin() {
    audioPlay("./appresources/wait_login.mp3", false,
		DeviceIoWrapper::getInstance()->led_on_logout, NULL, NULL);
}
void SoundController::network_fail_invalid_password(){
    audioPlay("./appresources/invalid_password.mp3", false, 
		DeviceIoWrapper::getInstance()->led_wrong_password, NULL, commonFinishCB);
}
void SoundController::network_fail_connect_router_failed(){
    audioPlay("./appresources/connect_router_failed.mp3", false, 
		DeviceIoWrapper::getInstance()->led_connection_routing_failed, NULL, commonFinishCB);
}

void SoundController::btModeTip() {
    audioPlay("./appresources/bt_mode_tip.mp3", true,
		NULL,NULL,NULL);
}

void SoundController::playTts(const std::string& content, bool needFocus, void (*callback)()) {
	for (auto observer : m_observers) {
		if (observer) {
			observer->playTts(content, needFocus, callback);
		}
	}
}


void SoundController::exTtsPlay(bool foucs) {
	audioPlay("/tmp/local_tts.mp3", foucs, NULL, NULL, NULL);
}

static void playTtsCallBack(){
	if(get_pids_by_name("ring.sh",NULL,1)){
		vfexec("ring.sh -n",1);
	}	
}

void SoundController::userTipSoundPlay(const std::string &url,int type,bool focus) {
	switch(type){
	case 0:audioPlay(url, focus, NULL, NULL, NULL);
		break;
	case 1:m_pcmPlayer->play(url,NULL, NULL, NULL);
		break;
	case 2:
		playTts(url,focus,playTtsCallBack);
		break;
	default:
		break;
	}	
}

}  // namespace application
}  // namespace duerOSDcsApp

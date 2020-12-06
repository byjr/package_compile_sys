#include "DcsSdk/AudioCallInterface.h"
#include "DCSApp/AudioCallProxy.h"
#include "DCSApp/SoundController.h"

extern std::condition_variable g_interruptCV;

namespace duerOSDcsApp {
namespace voip {
using namespace duerOSDcsSDK::sdkInterfaces;

AudioCallProxy * AudioCallProxy::m_AudioCallProxy = nullptr;
std::shared_ptr<AudioCallObserverInterface> AudioCallProxy::m_AudoiCallObserver = NULL;

AudioCallProxy::AudioCallProxy(){
	cdb_set_int("$audioCall_workStatus",0);
}


AudioCallProxy* AudioCallProxy::getInstance(){
	if(!m_AudioCallProxy){
		m_AudioCallProxy = new AudioCallProxy;
	}	
	return m_AudioCallProxy;
}

void AudioCallProxy::initiatecallByName(const std::string& name) {
	trc(__func__);
	if(m_AudoiCallObserver){
		m_AudoiCallObserver->onCallStartedEvent();
	}
}
void AudioCallProxy::initiatecallByNumber(const std::string& number) {
	trc(__func__);
	if(m_AudoiCallObserver){
		m_AudoiCallObserver->onCallStartedEvent();
	}
}
void AudioCallProxy::selectCall(std::vector<std::string>& numbers) {
	trc(__func__);
	if(m_AudoiCallObserver){
		m_AudoiCallObserver->onCallStartedEvent();
	}
}

void AudioCallProxy::startedCall(){
	if(m_AudoiCallObserver){trc(__func__);
		m_AudoiCallObserver->onCallStartedEvent();
		cdb_set_int("$audioCall_workStatus",2);//主叫已发起
	}
}

void AudioCallProxy::pickUpCall() {
	if(m_AudoiCallObserver){trc(__func__);
		m_AudoiCallObserver->onCallPickedUpEvent();
		cdb_set_int("$audioCall_workStatus",3);//接听来电
	}
}

void AudioCallProxy::hangUpCall() {	
	if(m_AudoiCallObserver){trc(__func__);
		m_AudoiCallObserver->onCallHungUpEvent();
		cdb_set_int("$audioCall_workStatus",0);//通话结束
	}
}

void AudioCallProxy::failedCall(){
	if(m_AudoiCallObserver){trc(__func__);
		m_AudoiCallObserver->onCallFailedEvent();
	}	
}
void AudioCallProxy::acceptedCall(){
	if(m_AudoiCallObserver){trc(__func__);
		m_AudoiCallObserver->onCallAcceptedEvent();
		cdb_set_int("$audioCall_workStatus",4);//去电被接听
	}
}
void AudioCallProxy::receivedCall(){
	if(m_AudoiCallObserver){trc(__func__);
		m_AudoiCallObserver->onCallReceivedEvent();
		cdb_set_int("$audioCall_workStatus",1);//来电铃响
	}
}
void AudioCallProxy::uploadLog() {
	if(m_AudoiCallObserver){trc(__func__);
		
	}
}

void AudioCallProxy::setObserver(std::shared_ptr<AudioCallObserverInterface> AudoiCallObserver) {
	trc("AudoiCallObserver=%d",AudoiCallObserver.get());
	m_AudoiCallObserver = AudoiCallObserver;
}

void* AudioCallProxy::tipSoundAudio(void* arg) {
	std::mutex g_interruptMTX;
	while(true){
		usleep(50);
		std::unique_lock<std::mutex> lk(g_interruptMTX);
		g_interruptCV.wait(lk);
//		AudioCallProxy::getInstance()->isCmccVoipBusy = true;
		err("is cmcc voip busying...");
		duerOSDcsApp::application::SoundController::getInstance()->AudioCallErrTip();
	}
}
}
}


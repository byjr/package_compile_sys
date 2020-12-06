#include <DCSApp/DeviceIoWrapper.h>
#include <DCSApp/DuerosFifoCmdTrigger.h>
#include "LoggerUtils/DcsSdkLogger.h"
#include "duer_link/duer_link_sdk.h"
#include "DcsSdk/ApplicationImplementation.h"
#include "MediaPlayer/Proxy/LocalTtsProxy.h"
#include "DCSApp/AudioCallProxy.h"
extern "C" {
	extern unixFifoOps_t *cmccVoipFifoPtr;
}
namespace duerOSDcsApp {
namespace application {
using namespace duerOSDcsApp::framework; 
using namespace duerLink;
using namespace duerOSDcsApp::voip;
using namespace duerOSDcsSDK::sdkInterfaces;

int cfgnet_handle(item_arg_t *arg){
//	char **argv=arg->argv;
	DeviceIoWrapper::getInstance()->callback(DeviceInput::KEY_ONE_LONG,NULL,0);
	return 0;
}
int logctrl_handle(item_arg_t *arg){	
	char **argv=(char **)arg->argv;
	assert_arg(1,-1);
	log_init(argv[1],NULL);
	war("%s:%s",__func__,argv[1]);
	return 0;
}
int logpath_handle(item_arg_t *arg){
	char **argv=(char **)arg->argv;
	assert_arg(1,-1);
	log_init(NULL,argv[1]);
	war("%s:%s",__func__,argv[1]);
	return 0;
}

int wakeup_handle(item_arg_t *arg){
	auto managerPtr=ApplicationManager::getInstance();	
	if(managerPtr && managerPtr->m_micWrapper){
		managerPtr->m_micWrapper->excuteWakeupResponse();
	}	
	return 0;
}
int keyevt_handle(item_arg_t *arg){
	char **argv=arg->argv;
	int value=0;
	if(!argv[1]){
		return -1;
	}
	DeviceInput key_value=(DeviceInput)atoi(argv[1]);
	if(argv[2]){
		value=atoi(argv[2]);
	}
	DeviceIoWrapper::getInstance()->callback(key_value,&value,0);
	return 0;
}
int micSync_handle(item_arg_t *arg){
//	DeviceIoWrapper::getInstance()->callback(DeviceInput::KEY_MIC_SYNC,NULL,0);
	return 0;
}

int vadTrdSet_handle(item_arg_t *arg){
	char **argv=arg->argv;
//	assert_arg(1,-1);
	auto managerPtr=ApplicationManager::getInstance();
	if(!managerPtr || !managerPtr->m_micWrapper){
		err("m_micWrapper isn't ready!");
		return -2;
	}
	AudioMicrophoneInterface* mWrapper = managerPtr->m_micWrapper.get();
	unsigned int vadThreshold=0;
	unsigned int startedConfirmCount=0;
	unsigned int finishedConfirmCount=0;
	unsigned int unitBytes=0;
	if(argv[1]){
		vadThreshold=10000*atoi(argv[1]);
		if(argv[2]){
			startedConfirmCount=atoi(argv[2]);
			if(argv[3]){
				finishedConfirmCount=atoi(argv[3]);
				if(argv[4]){
					unitBytes=atoi(argv[4]);
				}	
			}
		}
	}
//	inf("vadThreshold=%d",vadThreshold);
//	inf("startedConfirmCount=%d",startedConfirmCount);
//	inf("finishedConfirmCount=%d",finishedConfirmCount);
	if(mWrapper){
		if(vadThreshold){
			mWrapper->vad_args.vadThreshold=vadThreshold;
		}
		if(startedConfirmCount){
			mWrapper->vad_args.startedConfirmCount=startedConfirmCount;
		}
		if(finishedConfirmCount){
			mWrapper->vad_args.finishedConfirmCount=finishedConfirmCount;
		}
		if(unitBytes){
			mWrapper->vad_args.unitBytes=unitBytes;
		}
		inf("vadThreshold=%d",mWrapper->vad_args.vadThreshold);
		inf("startedConfirmCount=%d",mWrapper->vad_args.startedConfirmCount);
		inf("finishedConfirmCount=%d",mWrapper->vad_args.finishedConfirmCount);
		inf("unitBytes=%d",mWrapper->vad_args.unitBytes);
	}
	return 0;
}

int uPlay_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);
	int type=argv[2]?atoi(argv[2]):-1;
	bool focus = argv[3]?(atoi(argv[3])?true:false):false;
	SoundController::getInstance()->userTipSoundPlay(argv[1],type,focus);
	return 0;	
}

int appLog_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);
	bool appLogCtrl=atoi(argv[1]);
	LOGGER_ENABLE(appLogCtrl);
	war("appLogCtrl:%d",appLogCtrl?1:0);
	return 0;	
}
int aToken_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);
	auto managerPtr=ApplicationManager::getInstance();
	if(!managerPtr){
		return -2;
	}
	int type = atoi(argv[1]);
	managerPtr->setGetAccessTokenType(type);
	std::string accessToken="";
	managerPtr->m_dcsSdk->setAccessToken(accessToken);
	inf("aToken_handle done :%d",type);
	return 0;	
}

int infLinkState_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);
	auto state =(duerLink::notify_network_status_type)atoi(argv[1]);
	inf("notify_network_status_type %d",atoi(argv[1]));
	auto DuerLinkWrapperPtr=DuerLinkWrapper::getInstance();
	if(!DuerLinkWrapperPtr){
		return -2;
	}
	DuerLinkWrapperPtr->notify_network_config_status(state);
	return 0;	
}

int voipPlay_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);
	auto managerPtr=ApplicationManager::getInstance();
	if(!managerPtr){
		return -2;
	}
	int type = atoi(argv[1]);
	inf("---------%s------type:%d",__func__,type);
	if(type){
		DeviceIoWrapper::getInstance()->isCmccVoipPlayIng = true;
		managerPtr->forceHoldFocus(true);
	}else{
		DeviceIoWrapper::getInstance()->isCmccVoipPlayIng = false;
		managerPtr->forceHoldFocus(false);
	}	
	return 0;	
}
int voipRec_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);
	auto managerPtr=ApplicationManager::getInstance();
	if(!managerPtr || !managerPtr->m_micWrapper){
		err("m_micWrapper isn't ready!");
		return -2;
	}
	int type = atoi(argv[1]);
	inf("---------%s------type:%d",__func__,type);
	if(type){
		managerPtr->m_micWrapper->cmccVoipRcordingstarted();
	}else{
		managerPtr->m_micWrapper->cmccVoipRcordingStoped();
	}
	return 0;	
}
int exTtsPlay_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);
	int foucs = atoi(argv[1]);
	SoundController::getInstance()->exTtsPlay(foucs?true:false);
	return 0;	
}
int voipRing_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);	
	auto managerPtr=ApplicationManager::getInstance();
	if(!managerPtr){
		return -2;
	}
	int type = atoi(argv[1]);
	inf("---------%s------type:%d",__func__,type);
	if(type){
		managerPtr->forceHoldFocus(true);
	}else{
		managerPtr->forceHoldFocus(false);
	}
	return 0;
}
int btMusic_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);
	DuerosFifoCmd * ptr = (DuerosFifoCmd *)arg->args;
	if(!ptr){
		return -2;
	}

	inf("cmd : %s",argv[1]);
	std::string cmd = argv[1];
	if(!cmd.compare("resume")){
		ptr->BluetoothPlayerImpl->btStartPlay();
	}else if(!cmd.compare("pause")){
		ptr->BluetoothPlayerImpl->btStopPlay();
	}else if(!cmd.compare("disc")){
		ptr->BluetoothPlayerImpl->btDisconnect();
	}
	return 0;	
}
int focus_handle(item_arg_t *arg){
	char **argv=arg->argv;
   	assert_arg(1,-1);	
   	auto managerPtr=ApplicationManager::getInstance();
	if(!managerPtr){
		return -2;
	}
	int type = atoi(argv[1]);
	inf("---------%s------type:%d",__func__,type);
	if(type){
		managerPtr->forceHoldFocus(true);
	}else{
		managerPtr->forceHoldFocus(false);
	}
	return 0;
}
int voipCall_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);
	inf("cmd : %s",argv[1]);
	std::string cmd = argv[1];
	if(!cmd.compare("started")){
		AudioCallProxy::getInstance()->startedCall();
	}else if(!cmd.compare("pickUp")){
		AudioCallProxy::getInstance()->pickUpCall();
	}else if(!cmd.compare("hangUpSelf")){
		AudioCallProxy::getInstance()->hangUpCall();
		SoundController::getInstance()->userTipSoundPlay("./appresources/hangup_self.mp3",0,true);
	}else if(!cmd.compare("hangUp")){	
		assert_arg(2,-2);
		int errCode = atoi(argv[2]);
		inf("hangUp errCode:%d",errCode);
		if(errCode <= 200 && errCode >= 0){
			SoundController::getInstance()->userTipSoundPlay("./appresources/hangup.mp3",0,true);
		}else{
			AudioCallProxy::getInstance()->failedCall();
			SoundController::getInstance()->userTipSoundPlay("./appresources/hungUpErr.mp3",0,true);
		}
		AudioCallProxy::getInstance()->hangUpCall();
	}else if(!cmd.compare("accepted")){
		AudioCallProxy::getInstance()->acceptedCall();
	}else if(!cmd.compare("received")){
		AudioCallProxy::getInstance()->receivedCall();
	}else if(!cmd.compare("uploadLog")){
		AudioCallProxy::getInstance()->uploadLog();
		SoundController::getInstance()->userTipSoundPlay("./appresources/voip_log_upload.mp3",0,true);
	}else if(!cmd.compare("checkHangUp")){
		if(cdb_get_int("$audioCall_workStatus",0)){
			AudioCallProxy::getInstance()->hangUpCall();
		}		
	}
	return 0;	
}
int wanip_handle(item_arg_t *arg){
	if(cdb_get_int("$duerPing_en",0)){
		DuerLinkWrapper::getInstance()->verifyNetworkStatus(true);
	}
	return 0;
}
int exKey_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);
	inf("cmd : %s",argv[1]);
	std::string cmd = argv[1];
	if(!cmd.compare("phoRej")){
		if(cdb_get_int("$audioCall_workStatus",0) == 1){//来电响铃中。。。
			user_fifo_write_fmt(cmccVoipFifoPtr,"phoneEvent 3");
			return 0;
		}
	}else if(!cmd.compare("phoAck")){
		int audioCall_workStatus = cdb_get_int("$audioCall_workStatus",0);
		if(audioCall_workStatus == 1){//来电响铃中。。。
			user_fifo_write_fmt(cmccVoipFifoPtr,"phoneEvent 2");//接听电话
			return 0;
		}else if(audioCall_workStatus > 1){//2:主叫回铃中；3：被叫接听中；4：主叫被接中
			user_fifo_write_fmt(cmccVoipFifoPtr,"phoneEvent 3");//已挂断
			return 0;
		}
	}
	return -1;
}
int setVol_handle(item_arg_t *arg){
	char **argv=arg->argv;
	assert_arg(1,-1);
	inf("volume : %s",argv[1]);
	int volume = atoi(argv[1]);
	DeviceIoWrapper::getInstance()->callback(DeviceInput::KEY_RT_VOLUME,&volume,0);
	return 0;
}
cmd_item_t fifo_cmd_tbl[]={
	ADD_HANDLE_ITEM(setVol,NULL)
	ADD_HANDLE_ITEM(exKey,NULL)
	ADD_HANDLE_ITEM(wanip,NULL)
	ADD_HANDLE_ITEM(voipCall,NULL) 	
	ADD_HANDLE_ITEM(focus,NULL) 
	ADD_HANDLE_ITEM(btMusic,NULL) 
//	ADD_HANDLE_ITEM(micSync,NULL) 
	ADD_HANDLE_ITEM(exTtsPlay,NULL)	
	ADD_HANDLE_ITEM(voipRing,NULL)
	ADD_HANDLE_ITEM(voipPlay,NULL)
	ADD_HANDLE_ITEM(voipRec,NULL)
	ADD_HANDLE_ITEM(aToken,NULL)
	ADD_HANDLE_ITEM(infLinkState,NULL)
	ADD_HANDLE_ITEM(appLog,NULL)
	ADD_HANDLE_ITEM(uPlay,NULL)
	ADD_HANDLE_ITEM(logpath,NULL)
	ADD_HANDLE_ITEM(logctrl,NULL)
	ADD_HANDLE_ITEM(cfgnet,NULL)
	ADD_HANDLE_ITEM(wakeup,NULL)
	ADD_HANDLE_ITEM(keyevt,NULL)
	ADD_HANDLE_ITEM(vadTrdSet,NULL)
	{0}
};

fifo_cmd_t fifo_cmd_args={
	.type=FIFO_CMD_UNSPECIFIED_FIFO,
	.path="/tmp/duer.fifo",
	.tbl=fifo_cmd_tbl
};

DuerosFifoCmd * DuerosFifoCmd::DuerosFifoCmdPtr = NULL;

DuerosFifoCmd* DuerosFifoCmd::getInstance(){		
	if(!DuerosFifoCmdPtr){
		DuerosFifoCmdPtr =  new DuerosFifoCmd(NULL);		
	}
	return  DuerosFifoCmdPtr;
}

DuerosFifoCmd::~DuerosFifoCmd(){

}

void *DuerosFifoCmd::DuerosFifoCmdProc(void *ptr) {
	user_fifo_read_proc(&fifo_cmd_args);
	err("user_fifo_read_proc failed!");
    return nullptr;
}

int DuerosFifoCmd::startDuerosFifoCmdProc(void *arg){
	pthread_t DuerosFifoCmd_threadId;	
	pthread_create(&DuerosFifoCmd_threadId, nullptr, DuerosFifoCmdProc, this);
	pthread_setname_np(DuerosFifoCmd_threadId,"DuerosFifoCmd");
	return 0;
}


DuerosFifoCmd::DuerosFifoCmd(    void* args){
	int ret=user_fifo_read_init(&fifo_cmd_args);
	if(ret<0){
		err("user_fifo_read_init failed!");
		return;
	}
	int i = 0;
	for(;fifo_cmd_tbl[i].cmd;i++){
		if(strcmp(fifo_cmd_tbl[i].cmd,"btMusic") == 0){
			fifo_cmd_tbl[i].args = this;
		}
	}
}

}
}


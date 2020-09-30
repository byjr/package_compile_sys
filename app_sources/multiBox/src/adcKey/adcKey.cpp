#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/prctl.h>
#include <dirent.h>
#include <dirent.h>
#include "adcKey.h"
#include <iostream>
AdcKey::AdcKey(AdcKeyPar *par):
	mInputFd{-1},
	mDevFd{-1},
	mChangeFull{false},
	mUsbPlugIn{false} {
	mPar = par;
	if(mPar->inputPath) {
		mInputFd = open(mPar->inputPath, O_RDONLY);
		if(mInputFd < 0) {
			s_err("open %s", mPar->inputPath);
			show_errno(0, "open");
			exit(-1);
		}
		s_inf("mPar->inputPath:%s", mPar->inputPath);
	} else if(mPar->inputName) {
		mInputFd = openInputByName(mPar->inputName);
		if(mInputFd < 0) {
			s_err("open %s", mPar->inputName);
			exit(-1);
		}
		s_inf("mPar->inputPath:%s", mPar->inputName);
	}
	mRFds.fd = mInputFd;
	mRFds.events = POLLIN;
	if(mPar->devPath) {
		mDevFd = open(mPar->devPath, O_RDONLY);
		if(mDevFd < 0) {
			s_err("open %s", mPar->devPath);
			show_errno(0, "open");
			exit(-1);
		}
	}
}
int AdcKey::start() {
	return 0;
}
int AdcKey::stop() {
	return 0;
}
int AdcKey::rgbLightUpdate() {
	if(mUsbPlugIn) {
		if(mChangeFull) { //green on
			system("GpioCtrl.sh set 510 1");
			system("GpioCtrl.sh set 511 0");
		} else { //red on
			system("GpioCtrl.sh set 510 0");
			system("GpioCtrl.sh set 511 1");
		}
	} else { //off
		system("GpioCtrl.sh set 510 1");
		system("GpioCtrl.sh set 511 1");
	}
	return 0;
}
int AdcKey::listen() {
	struct input_event event;
	for(;;) {
		int res = poll(&mRFds, 1, -1);
		if(res == 0) {
			continue;
		}
		if (res < 0) {
			if(errno == EINTR) {
				continue;
			}
			show_errno(0, "poll");
			continue;
		}
		res = read(mInputFd, &event, sizeof(event));
		if(res <= 0) {
			show_errno(0, "read");
			continue;
		}
		s_dbg("type=%d,code:%d,value=%d", event.type, event.code, event.value);
		if (event.type == EV_KEY) {
			if(event.code == (int)keyCode::ADC_KEY_ACT) {
				if(event.value == (int)keyAct::KEY_PRESS) {

				} else if(event.value == (int)keyAct::KEY_RELEASE) {
					system("keyRelease.sh");
				}
			} else if(event.code == (int)keyCode::LOCK_STATE) {
				if(event.value == 1) { //lock
					s_inf("lock!");
				} else if(event.value == 0) { //unlock
					s_inf("unlock!");
				}
			} else if(event.code == (int)keyCode::ADC_ABMIN_ACT) {
				if(event.value == 1) { //Turn off the supplementary light
					system("GpioCtrl.sh set 105 0");//
				} else if(event.value == 0) { //Turn on the supplementary light
					system("GpioCtrl.sh set 105 1");//
				}
			} else if(event.code == (int)keyCode::PIR1_ACT) {
				if(event.value == 1) {
					s_inf("pir1_near!");
				} else if(event.value == 0) {
					s_inf("pir1_far!");
				}
			} else if(event.code == (int)keyCode::PIR2_ACT) {
				if(event.value == 1) {
					s_inf("pir2_near!");
				} else if(event.value == 0) {
					s_inf("pir2_far!");
				}
			} else if(event.code == (int)keyCode::USB_PLUG_ACT) {
				if(event.value == 1) {
					s_inf("usb_plug_in!");
					mUsbPlugIn = true;
				} else if(event.value == 0) {
					s_inf("usb_plug_out!");
					mUsbPlugIn = false;
				}
				rgbLightUpdate();
			} else if(event.code == (int)keyCode::CHANGE_STATE) {
				if(event.value == 1) {
					s_inf("change_full!");
					mChangeFull = true;
				} else if(event.value == 0) {
					s_inf("changing!");
					mChangeFull = false;
				}
				rgbLightUpdate();
			} else if(event.code == (int)keyCode::WAKE_KEY_ACT) {
				if(event.value == 1) {
					s_inf("wake key suspend!");
				} else if(event.value == 0) {
					s_inf("wake key wakeup!");
				}
			}
		} else {}
	}
	return 0;
}

#define mkmac(x) #x
#define mkstr(x) mkmac(x)
#define GET_OPT_VAL(opt) ({\
	std::string opt##st(mkstr(opt=));\
	keyIdx = raw.find(opt##st);\
	if(keyIdx != std::string::npos){\
		valIdx = raw.find_first_of(" \n",keyIdx+opt##st.size());\
		if(valIdx != std::string::npos){\
			opt = atoi(raw.substr(keyIdx+opt##st.size(),valIdx).data());\
		}\
	}\
	if(raw[valIdx] == '\n'){\
		return 0;\
	}\
})

int AdcKey::getCmd(int &cmd, int &code, int &val) {
	char buf[1024];
	std::string raw;
	std::size_t keyIdx = std::string::npos;
	std::size_t valIdx = std::string::npos;
	cmd = -1;
	code = -1;
	val = -1;
	memset(buf, 0, sizeof(buf));
	clog(Hred, __func__);
	char *res = fgets(buf, sizeof(buf), stdin);
	clog(Hred, __func__);
	if(!res) {
		s_err("fgets");
		return -1;
	}
	raw = buf;
	GET_OPT_VAL(cmd);
	GET_OPT_VAL(code);
	GET_OPT_VAL(val);
	// do{
	// memset(buf,0,sizeof(buf));
	// char *res = fgets(buf,sizeof(buf),stdin);
	// if(!res){
	// s_err("fgets");
	// return -1;
	// }
	// raw = buf;
	// std::string cmdst("cmd=");
	// keyIdx = raw.find(cmdst);
	// if(keyIdx != std::string::npos){
	// valIdx = raw.find_first_of(" \n",keyIdx+cmdst.size());
	// clog(Hred,"valIdx=%lu",valIdx);
	// if(valIdx != std::string::npos){
	// cmd = atoi(raw.substr(keyIdx+cmdst.size(),valIdx).data());
	// clog(Hred,"cmd=%d",cmd);
	// }
	// }
	// if(raw[valIdx] == '\n'){
	// return -1;
	// }

	// std::string codest("code=");
	// keyIdx = raw.find(codest);
	// if(keyIdx != std::string::npos){
	// valIdx = raw.find_first_of(" \n",keyIdx+codest.size());
	// if(valIdx != std::string::npos){
	// code = atoi(raw.substr(keyIdx+codest.size(),valIdx).data());
	// }
	// }
	// if(raw[valIdx] == '\n'){
	// return -1;
	// }

	// std::string valst("val=");
	// keyIdx = raw.find(valst);
	// if(keyIdx != std::string::npos){
	// valIdx = raw.find_first_of(" \n",keyIdx+valst.size());
	// if(valIdx != std::string::npos){
	// val = atoi(raw.substr(keyIdx+valst.size(),valIdx).data());
	// }
	// }
	// if(raw[valIdx] == '\n'){
	// return -1;
	// }
	// }while(0);
	if(cmd < 0) {
		return -1;
	}
	return 0;
}
int AdcKey::devRead() {
	mRdTrd = std::thread([this]() {
		int cmd, code, val, res = -1;
		for(;;) {
			if(getCmd(cmd, code, val) < 0) {
				continue;
			}
			clog(Hred, "cmd:%d,code:%d,val:%d", cmd, code, val);
			switch(cmd) {
			case 0://读出
				res = val > 0 ? ioctl(mDevFd, 0, val) : ioctl(mDevFd, 0, code);
				if(res < 0) {
					s_err("ioctl get val failed!");
				} else {
					s_inf("ioctl got val:%d", res);
				}
				break;
			case 1://设置阈值
				res = ioctl(mDevFd, 1, val);
				if(res < 0) {
					s_err("ioctl get set failed!");
				}
				break;
			default:
				s_err("unvaild cmd");
				break;
			}
		}
		return 0;
	});
}
AdcKey::~AdcKey() {
	close(mInputFd);
}

AdcKeyPar::AdcKeyPar():
	inputPath{NULL},
	inputName{"rk29-keypad"},
	devPath{NULL} {}

static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-i [inputPath]\n");
	printf("\t-n [inputName]\n");
	printf("\t-D [devPath]\n");
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}

#include <getopt.h>
int AdcKey_main(int argc, char **argv) {
	showCompileTmie(argv[0], s_war);
	AdcKeyPar adcKeyPar;
	int opt = -1;
	while((opt = getopt_long_only(argc, argv, "n:i:D:l:p:h", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'i':
			adcKeyPar.inputPath = optarg;
			break;
		case 'n':
			adcKeyPar.inputName = optarg;
			break;
		case 'D':
			adcKeyPar.devPath = optarg;
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	//打印编译时间
	s_inf("%s started...", __func__);
	auto adcKey = new AdcKey(&adcKeyPar);
	if(!adcKey) {
		s_err("new AdcKey");
		return -1;
	}
	if(adcKeyPar.devPath) {
		adcKey->devRead();
	}
	adcKey->listen();
	delete adcKey;
	s_inf("%s exited!", __func__);
	return 0;
}
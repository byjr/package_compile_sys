#ifndef ___ADC_KEY_H_H__
#define ___ADC_KEY_H_H__
#include <thread>
#include <lzUtils/base.h>
enum class keyCode {
	KEY_CODE_MIN 		= -1,
	ADC_KEY_ACT 		= 10,
	ADC_ABMIN_ACT 		= 11,
	USB_PLUG_ACT  		= 116,
	WAKE_KEY_ACT		= 143,
	CHANGE_STATE 		= 256,
	LOCK_STATE	 		= 257,
	PIR1_ACT			= 258,
	PIR2_ACT			= 259,
	KEY_CODE_MAX
};
enum class keyAct {
	KEY_ACT_MIN = -1,
	KEY_PRESS,
	KEY_RELEASE,
	LEY_CONTINUE,
	KEY_ACT_MAX
};
class AdcKeyPar {
public:
	const char *inputPath;
	const char *inputName;
	const char *devPath;
	AdcKeyPar();
};
class AdcKey {
	AdcKeyPar *mPar;
	int mInputFd;
	int mDevFd;
	struct pollfd mRFds;
	bool mChangeFull;
	bool mUsbPlugIn;
	std::thread mRdTrd;
	int getCmd(int &cmd, int &code, int &val);
public:
	AdcKey(AdcKeyPar *par);
	int start();
	int stop();
	int rgbLightUpdate();
	int listen();
	int devRead();
	~AdcKey();
};
#endif
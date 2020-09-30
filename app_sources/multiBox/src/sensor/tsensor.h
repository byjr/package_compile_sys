#ifndef __TSENSOR_H__
#define __TSENSOR_H__
#include <lzUtils/base.h>
#define TEMPERATURE_IOCTL_MAGIC 't'
#define TEMPERATURE_IOCTL_GET_ENABLED 		_IOR(TEMPERATURE_IOCTL_MAGIC, 1, int *)
#define TEMPERATURE_IOCTL_ENABLE 		_IOW(TEMPERATURE_IOCTL_MAGIC, 2, int *)
#define TEMPERATURE_IOCTL_DISABLE       	_IOW(TEMPERATURE_IOCTL_MAGIC, 3, int *)
#define TEMPERATURE_IOCTL_SET_DELAY       	_IOW(TEMPERATURE_IOCTL_MAGIC, 4, int *)
class TSensorPar {
public:
	const char *devPath;
	const char *inputName;
};
class TSensor {
	TSensorPar *mPar;
	int mDevFd;
	int mInputFd;
	struct pollfd mRFds;
public:
	TSensor(TSensorPar *par) {
		mPar = par;
		mDevFd = open(mPar->devPath, O_RDWR);
		if(mDevFd < 0) {
			s_err("open %s", mPar->devPath);
			exit(-1);
		}
		s_inf("mPar->devPath:%s", mPar->devPath);
		mInputFd = openInputByName(mPar->inputName);
		if(mInputFd < 0) {
			s_err("open %s", mPar->inputName);
			exit(-1);
		}
		s_inf("mPar->inputName:%s", mPar->inputName);
		mRFds.fd = mInputFd;
		mRFds.events = POLLIN;
	}
	int start() {
		unsigned int val = 1;
		s_inf("val=%d", val);
		int res = ioctl(mDevFd, TEMPERATURE_IOCTL_ENABLE, &val);
		if(res < 0) {
			s_err("%s TEMPERATURE_IOCTL_ENABLE", __func__);
			return -1;
		}
		return 0;
	}
	int stop() {
		unsigned int val = 0;
		s_inf("val=%d", val);
		int res = ioctl(mDevFd, TEMPERATURE_IOCTL_ENABLE, &val);
		if(res < 0) {
			s_err("%s TEMPERATURE_IOCTL_ENABLE", __func__);
			return -1;
		}
		return 0;
	}
	int listen() {
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
			s_inf("tempreture:%d,code=%d,type=%d", event.value, event.code, event.type);
			if (event.type == EV_ABS) {
				if (event.code == ABS_THROTTLE) {
					s_inf("tempreture:%d", event.value);
					continue;
				}
				s_dbg("invailed event.code");
			}
			continue;
			s_dbg("invailed event.type");
		}
		return 0;
	}
	int devRead() {
		unsigned long status = 0;
		int res = ioctl(mInputFd, TEMPERATURE_IOCTL_GET_ENABLED, &status);
		if(res < 0) {
			show_errno(0, "ioctl TEMPERATURE_IOCTL_GET_ENABLED");
			return -1;
		}
		s_inf("status=%lu", status);
		return 0;
	}
	~TSensor() {
		close(mInputFd);
		close(mDevFd);
	}
};
#endif
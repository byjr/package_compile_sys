#ifndef __PSENSOR_H__
#define __PSENSOR_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <unistd.h>
#include <lzUtils/base.h>
class PSensorPar {
public:
	const char *devPath;
	const char *inputPath;
};
class PSensor {
	PSensorPar *mPar;
	int mDevFd;
	int mInputFd;
	struct msghdr mMsg;
	struct sockaddr_nl sa;
	struct iovec iov;
	char mBuf[1024];
	int rawsocketCreate() {
		memset(&sa, 0, sizeof(sa));
		sa.nl_family = PF_NETLINK;
		sa.nl_groups = NETLINK_KOBJECT_UEVENT;
		sa.nl_pid = getpid();
		memset(&mMsg, 0, sizeof(mMsg));
		iov.iov_base = (void *)mBuf;
		iov.iov_len = sizeof(mBuf);
		mMsg.msg_name = (void *)&sa;
		mMsg.msg_namelen = sizeof(sa);
		mMsg.msg_iov = &iov;
		mMsg.msg_iovlen = 1;
		mInputFd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
		if(mInputFd == -1) {
			show_errno(0, "socket");
			return -1;
		}
		int res = bind(mInputFd, (struct sockaddr *)&sa, sizeof(sa));
		if(res < 0) {
			show_errno(0, "bind");
			return -1;
		}
		return mInputFd;
	}
public:
	PSensor(PSensorPar *par) {
		mPar = par;
		mDevFd = open(mPar->devPath, O_RDONLY);
		if(mDevFd < 0) {
			s_err("open %s", mPar->devPath);
			return;
		}
		mInputFd = rawsocketCreate();
		if(mInputFd < 0) {
			s_err("open %s", "NETLINK_KOBJECT_UEVENT");
			return;
		}
	}
	int start() {
		return 0;
	}
	int stop() {
		return 0;
	}
	int listen() {
		for(;;) {
			memset(mBuf, 0, sizeof(mBuf));
			int res = recvmsg(mInputFd, &mMsg, 0);
			if(res < 0) {
				show_errno(0, "socket");
				continue;
			} else if(res < 32 || res > sizeof(mBuf)) {
				s_war("invalid message");
				continue;
			}
			for(int i = 0; i < res; i++) {
				if(mBuf[i] == '\0') {
					mBuf[i] = ' ';
				}
			}
			s_inf("mBuf=%s", mBuf);
		}
		return 0;
	}
	int devRead() {
		long state = 0;
		int res = read(mDevFd, &state, sizeof(state));
		if(res < 0) {
			s_err("read state");
			return -1;
		}
		s_inf("state val:%ld", state);
		return 0;
	}
	~PSensor() {
		close(mInputFd);
		close(mDevFd);
	}
};
#endif
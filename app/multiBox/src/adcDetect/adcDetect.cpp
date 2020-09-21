#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <lzUtils/base.h>
static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-i [mIntavalMs]\n");
	printf("\t-c [mSampleCount]\n");
	printf("\t-v [mThreshold]\n");
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}
#include <getopt.h>
int adcDetect_main(int argc, char **argv) {
	showCompileTmie(argv[0], s_war);
	size_t mIntavalMs = 100;
	size_t mSampleCount = 1000;
	size_t mThreshold = 230;
	int opt = -1;
	while((opt = getopt_long_only(argc, argv, "v:i:c:l:p:h", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'i':
			mIntavalMs = atoi(optarg);
			break;
		case 'c':
			mSampleCount = atoi(optarg);
		case 'v':
			mThreshold = atoi(optarg);
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	//打印编译时间
	char buf[64] = {0};
	char *mPath = "/sys/bus/iio/devices/iio:device0/in_voltage4_raw";
	int mOpenFlag = O_RDONLY | O_NONBLOCK;
	s_inf("%s started...", __func__);
	char curIsHeight = 0;
	int i = 0;
	for(; i < mSampleCount; i ++) {
		int fd = 0;
		do {
			fd = open(mPath, mOpenFlag);
			if(fd < 0) {
				show_errno(0, "open");
				return -1;
			}
			int res = read(fd, buf, sizeof(buf));
			if(res < 0) {
				show_errno(0, "read");
				continue;
			}
			if(res < 1) {
				continue;
			}
			int val = atoi(buf);
			// showHexBuf(buf,res);
			s_dbg("get value:%u,curIsHeight:%d", val, curIsHeight);
			if(val < mThreshold && !curIsHeight) {
				curIsHeight = 1;
				system("GpioCtrl.sh set 105 1");
				s_inf("GpioCtrl.sh set 105 1");
			} else if(val >= mThreshold && curIsHeight) {
				curIsHeight = 0;
				system("GpioCtrl.sh set 105 0");
				s_inf("GpioCtrl.sh set 105 0");
			} else {
				curIsHeight = val < mThreshold ? 0 : 1;
			}
		} while(0);
		close(fd);
		s_dbg("i=%d", i);
		usleep(1000 * mIntavalMs);
		memset(buf, 0, sizeof(buf));
	}
	s_inf("%s exited!", __func__);
	return 0;
}
#include <dirent.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/prctl.h>
#include "psensor.h"
static PSensor *g_PSensor = NULL;
static void sig_handle(int sig) {
	switch(sig) {
	case SIGTERM:
		g_PSensor->stop();
		s_war("gsensor_exit!!!");
		exit(0);
		break;
	case SIGINT:
		g_PSensor->stop();
		s_war("gsensor_exit!!!");
		exit(0);
		break;
	default:
		break;
	}
}
// #include "parameter.h"
static int help_info(int argc, char *argv[]) {
	s_err("%s help:\n", get_last_name(argv[0]));
	s_err("\t-l [logLvCtrl]\n");
	s_err("\t-p [logPath]\n");
	s_err("\t-h show help\n");
	return 0;
}
#include <getopt.h>
int PSensor_main(int argc, char **argv) {
	showCompileTmie(argv[0], s_war);
	PSensorPar psPar = {
		.devPath = "/sys/devices/pirsensor_gt203s.22/pirsensor_gt203s",
		.inputPath = "/dev/input/event2",
	};
	int opt = -1;
	while((opt = getopt_long_only(argc, argv, "d:i:l:p:h", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'd':
			psPar.devPath = optarg;
			break;
		case 'i':
			psPar.inputPath = optarg;
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	auto mPSensor = new PSensor(&psPar);
	if(!mPSensor) {
		s_err("new PSensor");
		return -1;
	}
	g_PSensor = mPSensor;
	mPSensor->listen();
	delete mPSensor;
	return 0;
}
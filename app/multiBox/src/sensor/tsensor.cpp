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
#include "tsensor.h"

// #include "parameter.h"
static int help_info(int argc, char *argv[]) {
	s_err("%s help:\n", get_last_name(argv[0]));
	s_err("\t-l [logLvCtrl]\n");
	s_err("\t-p [logPath]\n");
	s_err("\t-h show help\n");
	return 0;
}
static TSensor *g_TSensor = NULL;
static void sig_handle(int sig) {
	switch(sig) {
	case SIGTERM:
		g_TSensor->stop();
		s_war("g_TSensor_exit!!!");
		exit(0);
		break;
	case SIGUSR1:
		g_TSensor->devRead();
		s_war("devRead!!!");
		break;
	case SIGINT:
		g_TSensor->stop();
		s_war("g_TSensor_exit!!!");
		exit(0);
		break;
	default:
		break;
	}
}
#include <getopt.h>
int TSensor_main(int argc, char **argv) {
	showCompileTmie(argv[0], s_war);
	TSensorPar tsPar = {
		.devPath = "/dev/temperature",
		.inputName = "temperature",
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
			tsPar.devPath = optarg;
			break;
		case 'i':
			tsPar.inputName = optarg;
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	signal(SIGTERM, sig_handle);
	signal(SIGINT, sig_handle);
	signal(SIGUSR1, sig_handle);
	auto mTSensor = new TSensor(&tsPar);
	if(!mTSensor) {
		s_err("new TSensor");
		return -1;
	}
	g_TSensor = mTSensor;
	mTSensor->start();
	mTSensor->listen();
	delete mTSensor;
	return 0;
}
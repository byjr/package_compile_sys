#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <lzUtils/base.h>
#include <getopt.h>
extern "C" {
#include <lzUtils/common/fd_op.h>
}
static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}
#include <getopt.h>
int lockCtrl_main(int argc, char **argv) {
	showCompileTmie(argv[0], s_war);
	int opt = -1;
	int mDelay =  400;
	while((opt = getopt_long_only(argc, argv, "t:l:p:h", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 't':
			mDelay =  atoi(optarg);
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	s_inf("mDelay=%d", mDelay);
	char *mPath = "/sys/class/gpio/gpio16/value";
	show_rt_time("lockCtrl start", s_inf);
	fd_write_file(mPath, "1", 1, "w");
	usleep(1000 * mDelay);
	fd_write_file(mPath, "0", 1, "w");
	show_rt_time("lockCtrl done", s_inf);
	return 0;
}
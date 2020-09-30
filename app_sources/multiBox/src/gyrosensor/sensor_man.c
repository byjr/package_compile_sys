#include <lzUtils/base.h>
extern int gsensor_init(void);
static int help_info(int argc, char *argv[]) {
	printf("\t-l [logLvCtrl]:eg:gsensor -l1111\n");
	printf("\t-p [logPath]gsensor -p /dev/console\n");
	printf("\t-h show help\n");
	return 0;
}
#include <getopt.h>
int gsensor_main(int argc, char *argv[]) {
	int opt  = -1;
	while ((opt = getopt_long_only(argc, argv, "l:p:th", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	//打印编译时间
	showCompileTmie(argv[0], s_war);
	int res = gsensor_init();
	if( res < 0) {
		s_err("gsensor_init failed!");
		return -1;
	}
	return -1;
}
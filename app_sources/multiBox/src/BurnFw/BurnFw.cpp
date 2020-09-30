#include <lzUtils/base.h>
extern "C" {
	int mtd_scan_partitions();
	int set_recovery();
};
static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-t [rFifo:wFifo]:use fifo to test logic.\n");
	printf("\t-m :Use stdin to simulate the Mastr.\n");
	printf("\t-b [baudRate]:Plz use 9600/38400/115200/1500000.\n");
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}
#include <getopt.h>
int BurnFw_main(int argc, char *argv[]) {
	int opt = 0;
	while ((opt = getopt_long_only(argc, argv, "L:b:mt:l:p:th", NULL, NULL)) != -1) {
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
	int count = get_pids_by_name(get_last_name(argv[0]), NULL, 2);
	s_war("count:%d", count);
	if(count > 1) {
		s_err("%s has runnig exit ...", get_last_name(argv[0]));
		exit(-1);
	}
	int res = mtd_scan_partitions();
	if(res < 0) {
		s_err("set_recovery res:%d", res);
		return -1;
	}
	s_inf("mtd_scan_partitions done!!");
	res = set_recovery();
	if(res < 0) {
		s_err("set_recovery res:%d", res);
		return -1;
	}
	s_inf("set_recovery done!!");
	system("reboot");
	return 0;
}
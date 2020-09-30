#include "RecvBc.h"
#include <lzUtils/base.h>
#include <vector>
#include <cppUtils/UrlContex/UrlContex.h>
using namespace cppUtils;
using namespace multlBox;
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

static void PackCallback(const char *data) {
	UrlContex url;
	if(!url.parse(data)) {
		s_err("%s/url.parse failed!!");
		return;
	}
	std::string ip = url.getParaVal("devIp");
	if(ip.size() > 0) {
		s_inf("recive the devIp:%s", ip.data());
		return ;
	}
}
#include <getopt.h>
int RecvBc_main(int argc, char *argv[]) {
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
	RecvBcPar mRecvBcPar;
	mRecvBcPar.parseCallback = &PackCallback;
	std::unique_ptr<RecvBc> mRecvBc(new RecvBc(&mRecvBcPar));
	if(!(mRecvBc.get() && mRecvBc->IsRunning())) {
		s_err("new mRecvBc failed!!");
		return -1;
	}
	mRecvBc->Loop();
	return 0;
}
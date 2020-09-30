#include "Broadcast.h"
#include <lzUtils/base.h>
#include <sstream>
#include <memory>
#include <vector>
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
#include <getopt.h>
int Broadcast_main(int argc, char *argv[]) {
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
	std::vector<char> ip(17,0);
	if(get_local_ip_by_name("wlan0",ip.data(),ip.size())){
		s_err("get_local_ip_by_name failed");
		return -1;
	}
	std::stringstream ctts;
	ctts << "GET /broadcast_info?";
	ctts << "devIp=" << ip.data();
	ctts << "\r\n\r\n";
	s_inf(ctts.str().data());
	BroadcastPar mBroadcastPar;
	mBroadcastPar.userCtt = ctts.str();
	std::unique_ptr<Broadcast> mBroadcast(new Broadcast(&mBroadcastPar));
	if(!(mBroadcast.get() && mBroadcast->IsRunning())){
		s_err("new mBroadcast failed!!");
		return -1;
	}
	mBroadcast->Loop();
	return 0;
}
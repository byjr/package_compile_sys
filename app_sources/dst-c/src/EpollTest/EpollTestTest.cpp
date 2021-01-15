#include <unistd.h>
#include <lzUtils/base.h>
#include "TcpServer.h"
int help_info(int argc, char *argv[]) {
	s_err("%s help:", get_last_name(argv[0]));
	s_err("\t-i [input path]");
	s_err("\t-o [output url]");
	s_err("\t-l [logLvCtrl]");
	s_err("\t-p [logPath]");
	s_err("\t-h show help");
	return 0;
}
int EpollTest_main(int argc, char *argv[]) {
	int opt  = -1;
	while ((opt = getopt(argc, argv, "l:p:h")) != -1) {
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
	std::shared_ptr<TcpServerPar> mTcpServerPar =
		std::make_shared<TcpServerPar>();
	std::unique_ptr<TcpServer> mTcpServer;
	mTcpServer = std::unique_ptr<TcpServer>(new TcpServer(mTcpServerPar));
	if(mTcpServer.get() == nullptr) {
		s_err("mTcpServer create failed!!");
		return -1;
	}
	return 0;
}
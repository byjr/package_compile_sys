#include "HttpCli.h"
#include <lzUtils/base.h>
#include <sstream>
#include <memory>
#include <cppUtils/UrlContex/UrlContex.h>
using namespace multlBox;
using namespace cppUtils;
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
static HttpCli* g_HttpCli = NULL;
static void PackCallback(const char* data){
	UrlContex url;
	if(!url.parse(data)){
		s_err("%s/url.parse failed!!");
		return;
	}
	std::string state = url.getParaVal("state");
	if(state == "recived"){
		s_inf("recive the state:%s",state.data());
		g_HttpCli->StopRun();
		return ;
	}	
}
#include <getopt.h>
int HttpCli_main(int argc, char *argv[]) {
	char* ssid = NULL;char* psk = NULL;
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
	std::stringstream ctts;
	ctts << "GET /send_wifi_info?";
	ctts << "&ssid=" << ssid;
	ctts << "&psk=" << psk;
	ctts << "\r\n\r\n";
	HttpCliPar mHttpCliPar;
	mHttpCliPar.userCtt = ctts.str();
	
	std::unique_ptr<HttpCli> mHttpCli(new HttpCli(&mHttpCliPar));
	if(!(mHttpCli.get() && mHttpCli->IsReady())){
		s_err("new mHttpCli failed!!");
		return -1;
	}
	g_HttpCli = mHttpCli.get();
	mHttpCli->Loop();
	return 0;
}
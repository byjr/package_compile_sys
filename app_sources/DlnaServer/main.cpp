#include <lzUtils/base.h>
#include <Dlna/DlnaDmrSdk.h>
#include <DlnaDmrOutputFfmpeg.h>
#include <getopt.h>
using namespace duerOSDcsApp::dueros_dlna;
int main(int argc, char** argv) {
	int opt = 0;
	while ((opt = getopt_long_only(argc, argv, "l:p:h", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		default: /* '?' */
			s_err("no para!");
		}
	}
	std::shared_ptr<IOutput> sp = std::make_shared<OutputFfmpeg>();
	DlnaDmrSdk dlnaDmrSdk;
	dlnaDmrSdk.add_output_module(sp);
	dlnaDmrSdk.start();	
	while(1){
		sleep(1);
	}
	return 0;
}

#include <cppUtils/CrcTool/CrcTool.h>
#include <lzUtils/common/fd_op.h>
#include <lzUtils/base.h>
#include <string>
#include <memory>
#include <vector>
#include <fstream>
using namespace cppUtils;
static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-c [rawData]\n");
	printf("\t-i [iPath]\n");
	printf("\t-h show help\n");
	return 0;
}
#include <getopt.h>
int CrcTool_main(int argc, char *argv[]) {
	std::string rawData("");
	const char *iPath = NULL;
	int opt = 0;
	const char *optstr = "l:p:c:i:h";
	while ((opt = getopt_long_only(argc, argv, optstr, NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'c':
			rawData = optarg;
			break;
		case 'i':
			iPath = optarg;
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	//打印编译时间
	showCompileTmie(argv[0], s_war);
	WaitOthersInstsExit(argv[0], 20);

	CrcToolPar CrcToolArgs;
	std::unique_ptr<CrcTool> mCrcTool(new CrcTool(&CrcToolArgs));
	if(!mCrcTool.get()) {
		s_err("new Uartd failed!");
		return -1;
	}
	if(rawData.size() > 0) {
		s_raw("%04x\n", mCrcTool->GetCrc32Val((CrcU8_t *)rawData.data(), rawData.size()));
	} else if(iPath) {
		std::vector<char>rdata;
		std::ifstream is(iPath,std::ios::binary);
		char ch=0;
		do{
			is.read(&ch,1);
			if(is.eof()){
				break;
			}
			rdata.push_back(ch);
		}while(!is.eof());
		// show_str_nbytes(rdata.data(), 32, s_war);
		s_raw("crc:0x%x\n", mCrcTool->GetCrc32Val((CrcU8_t *)rdata.data(), rdata.size()));
	}
	//-----------------------------
	return 0;
}
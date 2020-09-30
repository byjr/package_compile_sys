#include <lzUtils/base.h>
#include <cppUtils/Base64Tool/Base64Tool.h>
#include <iostream>
#include <memory>
using namespace cppUtils;
static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-i [iPath]\n");
	printf("\t-o [oPath]\n");
	printf("\t-s [eniBytes]\n");
	printf("\t-m [workMode]:0:nothing to do 1:read file ro encode 2:read file to decode\n");
	printf("\t\t3:use -c pass string to encode 4:use -c pass string to decode\n");
	printf("\t-c [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}
#include <getopt.h>
int Base64Tool_main(int argc, char *argv[]) {
	std::string ExternStr;
	const char *optstr = "c:i:o:s:m:l:p:h";
	int opt = -1;
	std::unique_ptr<Base64ToolPar> mBase64ToolPar(new Base64ToolPar());
	while((opt = getopt_long_only(argc, argv, optstr, NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'o':
			mBase64ToolPar->oPath = optarg;
			break;
		case 'i':
			mBase64ToolPar->iPath = optarg;
			break;
		case 's':
			mBase64ToolPar->eniBytes = atoi(optarg);
			break;
		case 'm':
			mBase64ToolPar->workMode = (Base64ToolMode_t)atoi(optarg);
			break;
		case 'c':
			ExternStr = optarg;
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	std::unique_ptr<Base64Tool> mBase64Tool(new Base64Tool(mBase64ToolPar.get()));
	if(mBase64ToolPar->workMode == Base64ToolMode_t::RF_ENC) {
		mBase64Tool->FileEncode();
	} else if(mBase64ToolPar->workMode == Base64ToolMode_t::RF_DEC) {
		mBase64Tool->FileDecode();
	} else if(mBase64ToolPar->workMode == Base64ToolMode_t::EI_ENC) {
		if(ExternStr.size() < 1) {
			s_err("Plz user -c to pass string for DataEncode!!!");
			return -1;
		}
		ssize_t res = mBase64Tool->DataEncode(ExternStr.data(), ExternStr.size());
		if(res < 0) {
			s_err("DataEncode failed!!");
			return -1;
		}
		mBase64Tool->GetEncOutBuf()[res] = 0;
		s_raw(mBase64Tool->GetEncOutBuf());
	} else if(mBase64ToolPar->workMode == Base64ToolMode_t::EI_DEC) {
		if(ExternStr.size() < 1) {
			s_err("Plz user -c to pass string(base64) for DataDecode!!!");
			return -1;
		}
		ssize_t res = mBase64Tool->DataDecode(ExternStr.data(), ExternStr.size());
		if(res < 0) {
			s_err("DataDecode failed!!");
			return -1;
		}
		mBase64Tool->GetDecOutBuf()[res] = 0;
		s_raw(mBase64Tool->GetDecOutBuf());
	}
	return 0;
}
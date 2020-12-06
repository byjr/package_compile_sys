#include <lzUtils/base.h>
#include <DlnaPlayerImpl.h>
#include "Dlna/DlnaDmrSdk.h"
#include "Dlna/DlnaDmrOutputFfmpeg.h"
#include "DlnaPlayerImpl.h"
using namespace duerOSDcsApp;
int main(int argc, char** argv) {
	auto dlnaPlayerImpl = mediaPlayer::DlnaPlayerImpl::create();
	if(!dlnaPlayerImpl){
		s_err("DlnaPlayerImpl create failed!");
		return -1;
	}
	std::shared_ptr<dueros_dlna::IOutput> sp = std::make_shared<dueros_dlna::OutputFfmpeg>();
	dueros_dlna::DlnaDmrSdk dlnaDmrSdk;
	dlnaDmrSdk.add_output_module(sp);
	dlnaDmrSdk.start();	
	while(1){
		sleep(1);
	}
	return 0;
}

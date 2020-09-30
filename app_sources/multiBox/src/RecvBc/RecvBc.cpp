#include "RecvBc.h"
#include "UrlContex/UrlContex.h"

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
static bool RecvBc_start() {
	RecvBcPar mRecvBcPar;
	mRecvBcPar.parseCallback = &PackCallback;
	std::unique_ptr<RecvBc> mRecvBc(new RecvBc(&mRecvBcPar));
	if(!(mRecvBc.get() && mRecvBc->IsRunning())) {
		s_err("new mRecvBc failed!!");
		return false;
	}
	mRecvBc->Loop();
	return true;
}
int main() {
	RecvBc_start();
}
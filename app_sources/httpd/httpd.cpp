#include <unistd.h>
#include <limits.h>
#include <getopt.h>
#include <lzUtils/base.h>
#include "DataBuffer.h"
#include "TcpServer.h"
namespace std {
	template<typename T, typename... Ts>
	std::unique_ptr<T> make_unq(Ts &&... params) {
		return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
	}
}
int main(int argc, char *argv[]) {
	int opt = 0;
	while ((opt = getopt(argc, argv, "l:p:h")) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'h':
			printf("%s help:\n", get_last_name(argv[0]));
			printf("\t-D [alsaDevice]\n");
			printf("\t-l [logLvCtrl]\n");
			printf("\t-d as a daemon excute\n");
			printf("\t-k killall %s\n", get_last_name(argv[0]));
			printf("\t-h show help\n");
			return 0;
		default: /* '?' */
			printf("\terr option!\n");
			return -1;
		}
	}
	auto mPlyBuf = std::make_shared<DataBuffer>(3);
	if(!mPlyBuf.get()) {
		s_err("");
		return -1;
	}
	auto mRecBuf = std::make_shared<DataBuffer>(3);
	if(!mRecBuf.get()) {
		s_err("");
		return -1;
	}
	auto servPar = std::make_unq<TcpServerPar>();
	servPar->plyBuf = mPlyBuf;
	servPar->recBuf = mRecBuf;

	auto serv = std::make_unq<TcpServer>(servPar);
	if(!(serv.get() && serv->isReady())) {
		s_err("serv create failed!!");
		return -1;
	}
	std::shared_ptr<DataBuffer> mBuf;
	for(int i = 0; i < 10000; i++) {
		char c = getchar();
		if( c == 'p') {
			s_war("start to dump ply data %d times ...", i);
			mBuf = std::move(mPlyBuf);
		} else if( c == 'r' ) {
			s_war("start to dump rec data %d times ...", i);
			mBuf = std::move(mRecBuf);
		} else {
			continue;
		}

		FILE *fp = fopen("111.pcm", "rb");
		if(!fp) {
			show_errno(0, "open");
			return -1;
		}
		int res = 0;
		data_ptr data;
		do {
			data = std::make_unq<data_unit>(PIPE_BUF);
			res = fread(data->data(), 1, data->size(), fp);
			if(res <= 0 ) {
				if(feof(fp)) {
					s_war("got eof!");
					break;
				}
				show_errno(0, "open");
				return -1;
			}
			if(!data->resize(res)) {
				s_err("");
				return -1;
			}
			mBuf->wbPush(data);
		} while(!feof(fp));
		fclose(fp);
	}
	return 0;
}
#include <unistd.h>
#include <lzUtils/base.h>
#include "TcpServer.h"
#include <limits.h>
int help_info(int argc, char *argv[]) {
	s_err("%s help:", get_last_name(argv[0]));
	s_err("\t-i [input path]");
	s_err("\t-o [output url]");
	s_err("\t-l [logLvCtrl]");
	s_err("\t-p [logPath]");
	s_err("\t-h show help");
	return 0;
}
int TcpServTest_main(int argc, char *argv[]) {
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

	std::unique_ptr<DataBuffer> mPlyBuf;
	mPlyBuf = std::unique_ptr<DataBuffer>(new DataBuffer(3));
	if(!mPlyBuf.get()) {
		s_err("");
		return -1;
	}
	std::unique_ptr<DataBuffer> mRecBuf;
	mRecBuf = std::unique_ptr<DataBuffer>(new DataBuffer(3));
	if(!mRecBuf.get()) {
		s_err("");
		return -1;
	}
	std::shared_ptr<TcpServerPar> mTcpServerPar;
	mTcpServerPar = std::make_shared<TcpServerPar>();
	mTcpServerPar->plyBuf = mPlyBuf.get();
	mTcpServerPar->recBuf = mRecBuf.get();

	std::unique_ptr<TcpServer> mTcpServer;
	mTcpServer = std::unique_ptr<TcpServer>(new TcpServer(mTcpServerPar));
	if(mTcpServer.get() == nullptr) {
		s_err("mTcpServer create failed!!");
		return -1;
	}
	DataBuffer *mBuf = nullptr;
	for(int i = 0; i < 10000; i++) {
		char c = getchar();
		if( c == 'p') {
			s_war("start to dump ply data %d times ...", i);
			mBuf = mTcpServerPar->plyBuf;
		} else if( c == 'r' ) {
			s_war("start to dump rec data %d times ...", i);
			mBuf = mTcpServerPar->recBuf;
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
			data = std::make_shared<data_unit>(PIPE_BUF);
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
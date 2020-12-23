#include <unistd.h>
#include <limits.h>
#include <getopt.h>
#include <lzUtils/base.h>
#include "DataBuffer.h"
#include "TcpServer.h"
int main(int argc,char* argv[]){
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
	std::unique_ptr<DataBuffer> mPlyBuf;
	mPlyBuf = std::unique_ptr<DataBuffer>(new DataBuffer(3));
	if(!mPlyBuf.get()){
		s_err("");
		return -1;
	}
	std::unique_ptr<DataBuffer> mRecBuf;
	mRecBuf = std::unique_ptr<DataBuffer>(new DataBuffer(3));
	if(!mRecBuf.get()){
		s_err("");
		return -1;
	}
	std::shared_ptr<TcpServerPar> servPar;
	servPar = std::make_shared<TcpServerPar>();
	servPar->plyBuf = mPlyBuf.get();
	servPar->recBuf = mRecBuf.get();
	
	std::unique_ptr<TcpServer> serv;
	serv = std::unique_ptr<TcpServer>(new TcpServer(servPar));
	if(!(serv.get() && serv->isReady())){
		s_err("serv create failed!!");
		return -1;
	}
	DataBuffer* mBuf = nullptr;
	for(int i=0;i< 10000;i++){
		char c = getchar();
		if( c == 'p'){
			s_war("start to dump ply data %d times ...",i);
			mBuf = servPar->plyBuf;
		}else if( c == 'r' ){
			s_war("start to dump rec data %d times ...",i);
			mBuf = servPar->recBuf;
		}else{
			continue;
		}
		
		FILE* fp = fopen("111.pcm","rb");
		if(!fp){
			show_errno(0,"open");
			return -1;
		}
		int res = 0;
		data_ptr data;
		do{
			data = std::make_shared<data_unit>(PIPE_BUF);
			res = fread(data->data(),1,data->size(),fp);
			if(res <=0 ){
				if(feof(fp)){
					s_war("got eof!");
					break;
				}
				show_errno(0,"open");
				return -1;
			}
			if(!data->resize(res)){
				s_err("");
				return -1;
			}
			mBuf->wbPush(data);
		}while(!feof(fp));
		fclose(fp);		
	}
	return 0;
}
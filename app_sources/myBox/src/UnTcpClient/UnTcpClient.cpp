// unordered_map::erase
#include <fstream>
#include <memory>
#include <thread>
#include <atomic>
#include <lzUtils/base.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <netdb.h>
static int help_info(int argc, char *argv[]) {
	s_err("%s help:", get_last_name(argv[0]));
	s_err("\t-i [input path]");
	s_err("\t-o [output url]");
	s_err("\t-l [logLvCtrl]");
	s_err("\t-p [logPath]");
	s_err("\t-h show help");
	return 0;
}
static int fd_set_flag(int fd, int flag) {
	int flags = fcntl(fd, F_GETFL, 0);
	if(flags < 0) {
		show_errno(0, "fcntl F_GETFL failed!");
		return -1;
	}
	int ret = fcntl(fd, F_SETFL, flags | flag);
	if(ret < 0) {
		show_errno(0, "fcntl F_SETFL failed!");
		return -2;
	}
	return 0;
}
class UnTcpClient{
    int cfd;
	bool initDoneFlag;
	std::atomic<bool> gotExitFlag;	
public:	
	UnTcpClient(const char* addr){
		initDoneFlag = false;
		gotExitFlag = false;		
        struct sockaddr_in my_addr;		
		cfd = socket(AF_INET, SOCK_STREAM, 0);
		if(cfd < 0){
			show_errno(0,"socket");
			return;
		}
		memset(&my_addr, 0, sizeof(struct sockaddr_in));
		my_addr.sin_family = AF_INET;
		my_addr.sin_port = htons(6666);
		if(inet_pton(AF_INET, "127.0.0.1", &my_addr.sin_addr) < 0){
			show_errno(0,"connect");
			return ;			
		}
		if (connect(cfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
			show_errno(0,"connect");
			return ;
		}
		if(fd_set_flag(cfd,O_NONBLOCK) < 0){
			return;
		}
		initDoneFlag = true;
	}
	~UnTcpClient(){
		if(cfd > 0){
			close(cfd);
		}
	}
	bool isInitDone(){
		return initDoneFlag;
	}
	bool run(){
		std::ifstream ifs("in.bin",std::ios::binary);
		if(!ifs.is_open()){
			show_errno(0,"ifs open");
			return false;		
		}
		std::vector<char> buffer(1024*4);
		for(;!gotExitFlag;){
			ifs.read(buffer.data(),buffer.size());
			if(ifs.gcount() <= 0){
				if(ifs.eof()){
					s_war("got eof!!");
					break;
				}
				show_errno(0,"ifs read");
				break;
			}
			int res = 0;
			size_t count  = 0;
			for(int i =0; i< 10 && count < buffer.size() && !gotExitFlag;){
				res = write(cfd,buffer.data()+count,buffer.size()-count);
				if(res <= 0){
					if(errno == EINTR){
						continue;
					}
					if(errno == EAGAIN){
						i++;
						s_war("retry ...!!");
						continue;
					}
					show_errno(0,"socket write!!!");
					break;
				}
				count += res;
			}
		}
		s_war("exit .....");
		ifs.close();
		s_war("exit !!!!!!");
		return true;
	}
};
int UnTcpClient_main(int argc,char* argv[]){
	int opt  =-1;
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
	std::unique_ptr<UnTcpClient> cli = 
		std::unique_ptr<UnTcpClient>(new UnTcpClient("/tmp/audioDump"));
	if(!cli->isInitDone()){
		s_err("UnTcpClient create failed!!!");
		return -1;
	}
	cli->run();
	s_war("exit main()...");
	cli.reset();
	s_war("exit main()!!!!");
    return 0;
}

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
class data_unit {
	char *data_ptr;
	size_t data_size;
	size_t space_size;
public:
	~data_unit() {
		if(data_ptr) {
			delete data_ptr;
		}
	}
	data_unit(size_t size, char ch) {
		data_ptr = new char[size];
		if(!data_ptr) {
			s_err("");
			return;
		}
		memset(data_ptr, ch, size);
		data_size = space_size = size;
	}
	data_unit(size_t size) {
		data_ptr = new char[size];
		if(!data_ptr) {
			s_err("");
			return;
		}
		data_size = space_size = size;
	}
	char *data() {
		return data_ptr;
	}
	size_t size() {
		return data_size;
	}
	size_t capacity() {
		return space_size;
	}
	bool resize(size_t size) {
		if(size > space_size) {
			auto ptr = new char[size];
			if(!ptr) {
				s_err("");
				return false;
			}
			memcpy(ptr, data_ptr, data_size);
			delete data_ptr;
			data_ptr = ptr;
			return true;
		}
		data_size = size;
		return true;
	}
};
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
static int help_info(int argc, char *argv[]) {
	s_err("%s help:", get_last_name(argv[0]));
	s_err("\t-i [input path]");
	s_err("\t-o [output url]");
	s_err("\t-l [logLvCtrl]");
	s_err("\t-p [logPath]");
	s_err("\t-h show help");
	return 0;
}

class UnTcpServer {
	int sfd, cfd;
	bool initDoneFlag;
	std::atomic<bool> gotExitFlag;
public:
	UnTcpServer() {
		initDoneFlag = false;
		gotExitFlag = false;
		struct sockaddr_in my_addr, peer_addr;
		sfd = socket(AF_INET, SOCK_STREAM, 0);
		if(sfd < 0) {
			show_errno(0, "socket");
			return;
		}
		memset(&my_addr, 0, sizeof(struct sockaddr_in));
		my_addr.sin_family = AF_INET;
		my_addr.sin_port = htons(6666);
		inet_pton(AF_INET, "127.0.0.1", &my_addr.sin_addr);

		int yes  = 1;
		if( setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1 ) {
			show_errno(0, "setsockopt");
			return;
		}
		if (bind(sfd, (struct sockaddr *) &my_addr,
				 sizeof(struct sockaddr_in)) == -1) {
			show_errno(0, "bind");
			return;
		}
		if (listen(sfd, 1) == -1) {
			show_errno(0, "listen");
			return;
		}
		socklen_t peer_addr_size = sizeof(struct sockaddr_in);
		cfd = accept(sfd, (struct sockaddr *) &peer_addr,
					 &peer_addr_size);
		if(cfd < 0) {
			show_errno(0, "socket");
			return;
		}
		if(fd_set_flag(cfd, O_NONBLOCK) < 0) {
			return;
		}
		initDoneFlag = true;
	}
	~UnTcpServer() {
		if(sfd > 0) {
			close(sfd);
		}
		if(cfd > 0) {
			close(cfd);
		}
	}
	bool isInitDone() {
		return initDoneFlag;
	}
	bool writeData(data_unit &buffer) {
		static FILE *fp = NULL;
		if(fp == NULL) {
			fp = fopen("out.bin", "wb");
			if(fp == NULL) {
				show_errno(0, " open");
				return false;
			}
		}
		if(fp == NULL) {
			return false;
		}
		ssize_t res = 0;
		size_t count  = 0;
		for(int i = 0; i < 10 && count < buffer.size() && !gotExitFlag;) {
			res = fwrite(buffer.data() + count, 1, buffer.size() - count, fp);
			if(res <= 0) {
				if(errno == EINTR) {
					continue;
				}
				if(errno == EAGAIN) {
					i++;
					s_war("retry ...!!");
					continue;
				}
				show_errno(0, "socket write!!!");
				break;
			}
			count += res;
		}
		fflush(fp);
		return true;
	}
	bool readData(data_unit &buffer) {
		int res = 0;
		size_t retryMax = 10;
		for(; !gotExitFlag;) {
			for(int i = 0; !gotExitFlag;) {
				res = read(cfd, buffer.data(), buffer.capacity());
				if(res == 0) {
					s_war("client closed connct!");
					buffer.resize(0);
					return true;
				}
				if(res < 0 ) {
					if(errno == EAGAIN || errno == EINTR) {
						usleep(10 * 1000);
						/* if(++i < retryMax) */{
							continue;
						}
						s_war("wait client timeout!");
						buffer.resize(0);
						return true;
					}
					show_errno(0, "read");
					return false;
				}
				break;
			}
			s_inf("res=%d", res);
			buffer.resize(res);
			return true;
		}
		return false;
	}
	bool run() {
		int res = 0;
		size_t got_bytes = 0;
		data_unit buffer(1024);
		for(; !gotExitFlag;) {
			if(!readData(buffer)) {
				return false;
			}
			if(buffer.size() == 0) {
				return true;
			}
			got_bytes += buffer.size();
			s_inf("got_bytes=%d", got_bytes);
			// s_inf("got dat:%s",buffer.data());
			if(!writeData(buffer)) {
				return false;
			}
		}
		return true;
	}
};
#include <pwd.h>
int UnTcpServer_main(int argc, char *argv[]) {
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
	std::unique_ptr<UnTcpServer> serv =
		std::unique_ptr<UnTcpServer>(new UnTcpServer());
	if(!serv->isInitDone()) {
		s_err("UnTcpClient create failed!!!");
		return -1;
	}
	uid_t uid = getuid();
	struct passwd *psw = getpwuid(uid);
	s_err("name:%s", psw->pw_name);
	return 0;
	serv->run();
	s_war("exit main()...");
	serv.reset();
	s_war("exit main()!!!!");
	return 0;
}

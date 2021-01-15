#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

constexpr int MAXLNE = 4096;

int inTcpSrv_main(int argc, char **argv) {
	int listenfd, connfd, n;
	struct sockaddr_in servaddr;
	char buff[MAXLNE];

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(6666);

	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	if (listen(listenfd, 10) == -1) {
		printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	printf("========waiting for client's request========\n");
	while (true) {
		if ((connfd = accept(listenfd, (struct sockaddr *)nullptr, nullptr)) == -1) {
			printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
			continue;
		}
		n = recv(connfd, buff, MAXLNE, 0);
		buff[n] = '\0';
		printf("recv msg from client: %s\n", buff);
		close(connfd);
	}

	close(listenfd);
	return 0;
}
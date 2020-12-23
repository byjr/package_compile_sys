#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
 
constexpr int MAXLNE = 4096;
const char * request = "GET /getPlyData HTTP/1.1\r\n\
Host: 192.168.113.12:10080\r\n\
Connection: keep-alive\r\n\
Upgrade-Insecure-Requests: 1\r\n\
User-Agent: curl/7.47.0\r\n\r\n";

int inTcpCli_main(int argc, char **argv) 
{
    int sockfd, n;
    struct sockaddr_in servaddr;
    char sendline[MAXLNE], recvline[MAXLNE];

 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(10080);
    if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %s\n", argv[1]);
        return 0;
    }
 
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    printf("send msg to server: \n");
    if (send(sockfd, request, strlen(request), 0) < 0) {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
	sleep(30);
    close(sockfd);
    return 0;
}
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
 
int inTcpCli_main(int argc, char **argv) 
{
    int sockfd, n;
    struct sockaddr_in servaddr;
    char sendline[MAXLNE], recvline[MAXLNE];
 
    if (argc != 2) {
        printf("usage: ./client <ipaddress>\n");
        return 0;
    }
 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %s\n", argv[1]);
        return 0;
    }
 
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    printf("send msg to server: \n");
    fgets(sendline, MAXLNE, stdin);
    if (send(sockfd, sendline, strlen(sendline), 0) < 0) {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    close(sockfd);
    return 0;
}
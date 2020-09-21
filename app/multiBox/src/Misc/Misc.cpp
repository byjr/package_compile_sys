#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <lzUtils/base.h>

int Misc_main(int argc,char *argv[]){
	int i=0;
	int sockfd;
	struct ifconf ifc;
	char buf[1024]={0};
	char ipbuf[20]={0};
	struct ifreq *ifr;
 
	ifc.ifc_len = 1024;
	ifc.ifc_buf = buf;
 
	if((sockfd = socket(AF_INET, SOCK_DGRAM,0))<0)
	{
	    printf("socket error\n");
		return -1;
	}
	ioctl(sockfd,SIOCGIFCONF, &ifc);
	ifr = (struct ifreq*)buf;
 
	for(i=(ifc.ifc_len/sizeof(struct ifreq)); i > 0; i--)
	{
		printf("net name: %s\n",ifr->ifr_name);
		inet_ntop(AF_INET,&((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr,ipbuf,20);
		printf("ip: %s \n",ipbuf);
		ifr = ifr +1;
	}
	return 0;
}

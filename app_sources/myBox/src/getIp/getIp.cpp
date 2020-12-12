#include <string>
#include <vector>
#include <lzUtils/base.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#define IP_SIZE (16)
// 获取本机ip
int get_local_ip(const char *eth_inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

    close(sd);
    return 0;
}
int getIp_main(int argc,char* argv[]){
	int opt  =-1;
	std::string if_name;
	while ((opt = getopt(argc, argv, "i:l:p:h")) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'i':
			if_name = optarg;
			break;			
		default: /* '?' */
			s_err("unsorport option!");
			return 0;
		}
	}
	std::vector<char>ipBuffer(IP_SIZE,0);
	if(get_local_ip(if_name.data(),ipBuffer.data()) < 0){
		s_err(" get_local_ip failed!");
		return -1;
	}
	s_war("ipaddr=%s",ipBuffer.data());
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <asm/types.h>
//该头文件需要放在netlink.h前面防止编译出现__kernel_sa_family未定义
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <unistd.h>
#include <lzUtils/base.h>

int MonitorNetlinkUevent() {
	int sockfd;
	struct sockaddr_nl sa;
	char buf[4096];
	struct iovec iov;
	struct msghdr msg;
	int i;

	memset(&sa, 0, sizeof(sa));
	sa.nl_family = PF_NETLINK;
	sa.nl_groups = NETLINK_KOBJECT_UEVENT;
	sa.nl_pid = getpid();
	memset(&msg, 0, sizeof(msg));
	iov.iov_base = (void *)buf;
	iov.iov_len = sizeof(buf);
	msg.msg_name = (void *)&sa;
	msg.msg_namelen = sizeof(sa);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	sockfd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if(sockfd == -1) {
		show_errno(0, "socket");
		return -1;
	}
	int res = bind(sockfd, (struct sockaddr *)&sa, sizeof(sa));
	if(res < 0) {
		show_errno(0, "bind");
		return -1;
	}
	for(;;) {
		memset(buf, 0, sizeof(buf));
		res = recvmsg(sockfd, &msg, 0);
		if(res < 0) {
			show_errno(0, "socket");
			continue;
		} else if(res < 32 || res > sizeof(buf)) {
			s_war("invalid message");
			continue;
		}
		for(i = 0; i < res; i++) {
			if(buf[i] == '\0') {
				buf[i] = ' ';
			}
		}
		s_inf("received %d bytes\n%s\n", res, buf);
		if(strstr(buf, "USB_STATE=DISCONNECTED")) {
			s_inf("DISCONNECTED------------");
			system("GpioCtrl.sh set 510 1");
			system("GpioCtrl.sh set 511 1");
		} else if(strstr(buf, "USB_STATE=CONNECTED")) {
			char rbuf[16] = {0};
			int ret = 0;
			my_popen_get((char *)&rbuf, sizeof(rbuf), "GpioCtrl.sh get 5");
			s_inf("res=%d", ret);
			ret = atoi(rbuf);
			if(ret) {
				s_inf("chage full-----------------");
				system("GpioCtrl.sh set 510 0");
				system("GpioCtrl.sh set 511 0");
			} else {
				s_inf("chaging----------------------");
				system("GpioCtrl.sh set 510 0");
				system("GpioCtrl.sh set 511 1");
			}
		}
	}
	return 0;
}
static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}
#include <getopt.h>
int hotplug_main(int argc, char **argv) {
	showCompileTmie(argv[0], s_war);
	int opt = -1;
	while((opt = getopt_long_only(argc, argv, "l:p:h", NULL, NULL)) != -1) {
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
	//打印编译时间
	showCompileTmie(argv[0], s_war);
	MonitorNetlinkUevent();
	return 0;
}
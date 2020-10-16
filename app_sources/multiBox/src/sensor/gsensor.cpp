/*
 * Rockchip App
 *
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 * Author: Wang RuoMing <wrm@rock-chips.com>
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gsensor.h"
#include <dirent.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/prctl.h>
#include <lzUtils/base.h>
// #include "parameter.h"

#define GSENSOR_FUN_NUM         128
#define GSENSOR_NAME_LENGTH     80
#define GSENSOR_DISABLE         0
#define GSENSOR_ENABLE          1

#define GSENSOR_IOCTL_MAGIC      'a'
#define GBUFF_SIZE               12    /* Rx buffer size */

/* IOCTLs for gsensor */
#define GSENSOR_IOCTL_INIT       _IO(GSENSOR_IOCTL_MAGIC, 0x01)
#define GSENSOR_IOCTL_CLOSE      _IO(GSENSOR_IOCTL_MAGIC, 0x02)
#define GSENSOR_IOCTL_START      _IO(GSENSOR_IOCTL_MAGIC, 0x03)
#define GSENSOR_IOCTL_RESET      _IO(GSENSOR_IOCTL_MAGIC, 0x04)
#define GSENSOR_IOCTL_COL_SET    _IO(GSENSOR_IOCTL_MAGIC, 0x10)

#define GSENSOR_IOCTL_GETDATA \
_IOR(GSENSOR_IOCTL_MAGIC, 0x08, char[GBUFF_SIZE+1])

#define GSENSOR_IOCTL_USE_INT1  _IOW(GSENSOR_IOCTL_MAGIC, 0x05, char)
#define GSENSOR_IOCTL_USE_INT2  _IOW(GSENSOR_IOCTL_MAGIC, 0x06, char)
#define GSENSOR_IOCTL_REG_R     _IOW(GSENSOR_IOCTL_MAGIC, 0x11, char)
#define GSENSOR_IOCTL_REG_W     _IOW(GSENSOR_IOCTL_MAGIC, 0x12, char)

#define GSDEV "/dev/mma8452_daemon"
#define ACCEL_RATIO (9.80665f / 1000000)



struct gsensor_send_data {
	int num;

	int (*send)(struct sensor_axis data);
};

struct gsensor_reg_wr {
	int addr;
	unsigned char *pdata;
};

struct gsensor {
	int gsfd;
	int gsinput;
	int gsthread_state;
	int gstate;
	int gsrelease_state;
	int axis_x_init;
	int axis_y_init;
	int axis_z_init;
	int gsfun_count;

	struct pollfd fds;
	struct sensor_axis gsdata;

	pthread_mutex_t gs_mutex;
};

static struct gsensor sensor;
static struct gsensor_send_data senddata[GSENSOR_FUN_NUM];

int gsensor_enable(int enable);
static int gsensor_create_thread(void);
static int gsensor_delete_thread(void);

void (*gsensor_event_call)(int cmd, void *msg0, void *msg1);

static int gsensor_getregnum(void) {
	int i;

	for (i = 0; i < GSENSOR_FUN_NUM; i++)
		if (senddata[i].num == 0)
			break;

	return (i < GSENSOR_FUN_NUM) ? (i + 1) : -1;
}

int gsensor_regsenddatafunc(int (*send)(struct sensor_axis data)) {
	int i;
	int num = -1;

	pthread_mutex_lock(&sensor.gs_mutex);

	num = gsensor_getregnum();

	if ((num < 1) || (num > GSENSOR_FUN_NUM)) {
		s_err("Gsensor register send function failure\n");
		pthread_mutex_unlock(&sensor.gs_mutex);
		return -1;
	}

	for (i = 0; i < GSENSOR_FUN_NUM; i++) {
		if (senddata[i].send == send) {
			s_err("Gsensor multiple register function failure\n");
			pthread_mutex_unlock(&sensor.gs_mutex);
			return -1;
		}
		if (senddata[i].num == 0)
			break;
	}
	if (i != num - 1) {
		s_err("Gsensor register send function failure\n");
		pthread_mutex_unlock(&sensor.gs_mutex);
		return -1;
	}

	if (senddata[num - 1].send == NULL) {
		s_err("Gsensor register send function success\n");
		senddata[num - 1].send = send;
		senddata[num - 1].num = num;
		sensor.gsfun_count++;
	} else {
		s_err("Gsensor register send function failure\n");
		pthread_mutex_unlock(&sensor.gs_mutex);
		return -1;
	}

	/* register success start gsensor thread */
	if (sensor.gsthread_state != 1) {
		sensor.gsthread_state = 1;
		gsensor_create_thread();
	}

	pthread_mutex_unlock(&sensor.gs_mutex);

	return num;
}

int gsensor_unregsenddatafunc(int num) {
	int i;

	pthread_mutex_lock(&sensor.gs_mutex);

	if ((num < 1) || (num > GSENSOR_FUN_NUM)) {
		s_err("Gsensor unregister send function failure\n");
		pthread_mutex_unlock(&sensor.gs_mutex);
		return -1;
	}

	if (senddata[num - 1].send != NULL) {
		s_err("Gsensor unregister send function success\n");
		senddata[num - 1].send = NULL;
		senddata[num - 1].num = 0;
		sensor.gsfun_count--;
	} else {
		s_err("Gsensor unregister send function failure\n");
		pthread_mutex_unlock(&sensor.gs_mutex);
		return -1;
	}

	/* unregister success stop gsensor thread */
	if (sensor.gsthread_state == 1) {
		for (i = 0; i < GSENSOR_FUN_NUM; i++)
			if (senddata[i].num != 0)
				break;

		if (i == GSENSOR_FUN_NUM) {
			s_err("Gsensor unregister delete thread\n");
			sensor.gsthread_state = 0;
			sensor.gsrelease_state = 1;
			/* disable gsensor */
			gsensor_enable(GSENSOR_DISABLE);
		}
	}
	pthread_mutex_unlock(&sensor.gs_mutex);

	return 0;
}

static int sensor_data_x, sensor_data_y, sensor_data_z;

void get_gsensor_data(int *x, int *y, int *z) {
	*x = sensor_data_x;
	*y = sensor_data_y;
	*z = sensor_data_z;
}

static int gsensor_readevent(struct sensor_axis *accdata, int count) {
	int ret = 0;
	int i = 0;
	int send_count = 0;
	struct input_event event;

	while (count) {
		if ((sensor.gsrelease_state == 1) ||
				(sensor.gstate == GSENSOR_DISABLE)) {
			ret = -1;
			break;
		}

		ret = poll(&sensor.fds, 1, 100);
		if (ret <= 0)
			continue;
		else
			ret = read(sensor.gsinput, &event, sizeof(event));
		if (ret < 0)
			break;

		if (event.type == EV_ABS) {
			if (event.code == ABS_X) {
				accdata->x = event.value * ACCEL_RATIO;
				sensor_data_x = event.value;
				sensor.axis_x_init = 1;
				s_inf("raw_x=%d", event.value);
			} else if (event.code == ABS_Y) {
				accdata->y = event.value * ACCEL_RATIO;
				sensor_data_y = event.value;
				sensor.axis_y_init = 1;
				s_inf("raw_y=%d", event.value);
			} else if (event.code == ABS_Z) {
				accdata->z = event.value * ACCEL_RATIO;
				sensor_data_z = event.value;
				sensor.axis_z_init = 1;
				s_inf("raw_z=%d", event.value);
			}
		} else if (event.type == EV_SYN) {
			count--;
			if ((sensor.axis_x_init != 1) ||
					(sensor.axis_y_init != 1) ||
					(sensor.axis_z_init != 1)) {
				count++;
				continue;
			}

			if (count <= 0) {
				/* send gsensor axis data */
				if (sensor.gsfun_count <= 0)
					return 1;

				for (i = 0; i < GSENSOR_FUN_NUM; i++) {
					if ((senddata[i].send != NULL) &&
							(senddata[i].num != 0)) {
						senddata[i].send(*accdata);
						send_count++;
					}
					if (sensor.gsfun_count == send_count) {
						send_count = 0;
						break;
					}
				}
				return 1;
			}
		} else {
			s_err("gsensor unknown event -----\n");
		}
	}

	return ret;
}

static void *gsensor_thread(void *arg) {
	int ret = 0;
	int init_sensor_data = 0;
	struct sensor_axis new_gs_data;

	prctl(PR_SET_NAME, "gsensor_thread", 0, 0, 0);

	/* enable gsensor */
	ret = gsensor_enable(GSENSOR_ENABLE);
	if (ret < 0) {
		s_err("gsensor enable failure\n");
		goto PTHREAD_EXIT;
	}

	memset(&new_gs_data, 0, sizeof(new_gs_data));

	while (!sensor.gsrelease_state) {
		/* get first gsensor data when gsensor work */
		if (init_sensor_data == 0) {
			ret = gsensor_readevent(&sensor.gsdata, 1);
			if (ret < 0) {
				continue;
			} else {
				init_sensor_data = 1;
				new_gs_data.x = sensor.gsdata.x;
				new_gs_data.y = sensor.gsdata.y;
				new_gs_data.z = sensor.gsdata.z;
			}
		}
		s_dbg("x=%d,y=%d,z=%d", new_gs_data.x, new_gs_data.y, new_gs_data.z);
		/* read gsensor data */
		ret = gsensor_readevent(&new_gs_data, 1);
		if (ret < 0)
			continue;
	}

PTHREAD_EXIT:
	sensor.gsrelease_state = 0;
	sensor.gsthread_state = 0;
	gsensor_delete_thread();

	return 0;
}

int gsensor_use_interrupt(int intr_num, int intr_st) {
	int ret = 0;
	int int_status = intr_st;

	if (sensor.gsfd < 0) {
		s_err("gsensor open fail, not use interrupt%d", intr_num);
		ret = -1;
		return ret;
	}

	switch (intr_num) {
	case GSENSOR_INT1:
		if (intr_st == GSENSOR_INT_START) {
			ret = ioctl(sensor.gsfd,
						GSENSOR_IOCTL_USE_INT1,
						&int_status);
			if (ret < 0) {
				s_err("set int%d status=%d failure\n",
					  intr_num, int_status);
			}
		} else {
			int_status = 0;
			ret = ioctl(sensor.gsfd,
						GSENSOR_IOCTL_USE_INT1,
						&int_status);
			if (ret < 0) {
				s_err("set int%d status=%d failure\n",
					  intr_num, int_status);
			}
		}
		break;
	case GSENSOR_INT2:
		if (intr_st == GSENSOR_INT_START) {
			ret = ioctl(sensor.gsfd,
						GSENSOR_IOCTL_USE_INT2,
						&int_status);
			if (ret < 0) {
				s_err("set int%d status=%d failure\n",
					  intr_num, int_status);
			}
		} else {
			int_status = 0;
			ret = ioctl(sensor.gsfd,
						GSENSOR_IOCTL_USE_INT2,
						&int_status);
			if (ret < 0) {
				s_err("set int%d status=%d failure\n",
					  intr_num, int_status);
			}
		}
		break;
	default:
		s_err("use gsensor int%d error\n", intr_num);
		ret = -1;
		break;
	}

	return ret;
}

int gsensor_enable(int enable) {
	int ret = -1;
	s_inf("%s,enable=%d", __func__, enable);
	if (enable) {
		ret = ioctl(sensor.gsfd, GSENSOR_IOCTL_START, &enable);
		if (ret < 0) {
			s_err("%s:gsensor enable failure", __func__);
			return -1;
		}
		sensor.gstate = GSENSOR_ENABLE;
	} else {
		ret = ioctl(sensor.gsfd, GSENSOR_IOCTL_CLOSE, &enable);
		if (ret < 0) {
			s_err("%s:gsensor disable failure", __func__);
			return -1;
		}
		sensor.gstate = GSENSOR_DISABLE;
	}

	return ret;
}

void gsensor_release(void) {
	if (sensor.gsfd >= 0) {
		close(sensor.gsfd);
		sensor.gsfd = -1;
	}

	if (sensor.gsinput >= 0) {
		close(sensor.gsinput);
		sensor.gsinput = -1;
	}

	pthread_mutex_destroy(&sensor.gs_mutex);
}

static int open_input(const char *inputname) {
	int fd = -1;
	const char *dirname = "/dev/input";
	char devname[512] = { 0 };
	char *filename;
	DIR *dir;
	struct dirent *de;

	dir = opendir(dirname);
	if (dir == NULL)
		return -1;
	strcpy(devname, dirname);
	filename = devname + strlen(devname);
	*filename++ = '/';
	while ((de = readdir(dir))) {
		if (de->d_name[0] == '.' &&
				(de->d_name[1] == '\0' ||
				 (de->d_name[1] == '.' && de->d_name[2] == '\0')))
			continue;
		strcpy(filename, de->d_name);
		s_inf("devname:%s", devname);
		fd = open(devname, O_RDONLY);
		if (fd >= 0) {
			char name[GSENSOR_NAME_LENGTH] = {0};

			if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1)
				name[0] = '\0';

			if (!strcmp(name, inputname)) {
				break;
			} else {
				close(fd);
				fd = -1;
			}
		}
	}
	closedir(dir);

	return fd;
}

static int gsensor_delete_thread(void) {
	pthread_detach(pthread_self());
	pthread_exit(0);
}

static int gsensor_create_thread(void) {
	int ret = 0;
	pthread_t tid;

	if (pthread_create(&tid, NULL, gsensor_thread, NULL)) {
		s_err("create gsensor_thread pthread failed\n");
		sensor.gsthread_state = 0;
		ret = -1;
	}

	return ret;
}

int gsensor_init(void) {
	char collision_level = 0;
	char parkingmonitor = 0;

	s_inf("%s\n", __func__);
	memset(&sensor, 0, sizeof(sensor));
	sensor.gsfd = -1;
	sensor.gsinput = -1;
	pthread_mutex_init(&sensor.gs_mutex, NULL);
	pthread_mutex_lock(&sensor.gs_mutex);

	sensor.gsfd = open(GSDEV, O_RDWR);
	if (sensor.gsfd < 0) {
		s_err("failed to open gsensor dev\n");
		goto GSENSOR_EXIT;
	}

	sensor.gsinput = open_input("gsensor");
	if (sensor.gsinput < 0) {
		s_err("failed to open gsensor input dev\n");
		close(sensor.gsfd);
		goto GSENSOR_EXIT;
	}

	sensor.fds.fd = sensor.gsinput;
	sensor.fds.events = POLLIN;

	/* gsensor driver state 0:off 1:on */
	memset(senddata, 0, sizeof(senddata));

	collision_level = 1;
	parkingmonitor = 1;

	/* start or stop gsensor */
	if ((parkingmonitor != 0) || (collision_level != 0)) {
		sensor.gsthread_state = 1;
		gsensor_create_thread();
	}
	pthread_mutex_unlock(&sensor.gs_mutex);

	return 0;

GSENSOR_EXIT:
	pthread_mutex_unlock(&sensor.gs_mutex);
	return -1;
}

int I2C_GSensor_Read(int addr, unsigned char *pdata) {
	int ret = 0;
	struct gsensor_reg_wr arg;

	arg.addr = addr;
	arg.pdata = pdata;
	if (sensor.gsfd < 0) {
		s_err("gsensor open fail,not use gsensor\n");
		ret = -1;
		return ret;
	}

	ret = ioctl(sensor.gsfd, GSENSOR_IOCTL_REG_R, &arg);
	if (ret < 0)
		s_err("GSENSOR_IOCTL_REG_R failure\n");

	return 0;
}

int I2C_GSensor_Write(int addr, unsigned char *pdata) {
	int ret = 0;
	struct gsensor_reg_wr arg;

	arg.addr = addr;
	arg.pdata = pdata;
	if (sensor.gsfd < 0) {
		s_err("gsensor open fail,not use gsensor\n");
		ret = -1;
		return ret;
	}

	ret = ioctl(sensor.gsfd, GSENSOR_IOCTL_REG_W, &arg);
	if (ret < 0)
		s_err("GSENSOR_IOCTL_REG_R failure\n");

	return 0;
}

int I2C_RegEventCallback(fpI2C_Event *EventCallBack) {
	s_inf(__func__);
	return 0;
}

static int help_info(int argc, char *argv[]) {
	s_err("%s help:\n", get_last_name(argv[0]));
	s_err("\t-l [logLvCtrl]\n");
	s_err("\t-p [logPath]\n");
	s_err("\t-h show help\n");
	return 0;
}

static void sig_handle(int sig) {
	switch(sig) {
	case SIGTERM:
		gsensor_enable(GSENSOR_DISABLE);
		s_war("gsensor_exit!!!");
		exit(0);
		break;
	case SIGINT:
		gsensor_enable(GSENSOR_DISABLE);
		s_war("gsensor_exit!!!");
		exit(0);
		break;
	default:
		break;
	}
}
#include <getopt.h>
int GSensor_main(int argc, char **argv) {
	showCompileTmie(argv[0], s_war);
	int opt = -1;
	while((opt = getopt_long_only(argc, argv, "v:i:c:l:p:h", NULL, NULL)) != -1) {
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
	signal(SIGTERM, sig_handle);
	signal(SIGINT, sig_handle);
	int res = gsensor_init();
	if(res < 0) {
		s_err("gsensor_init");
		return -1;
	}
	for(;;) {
		pause();
	}
	return 0;
}
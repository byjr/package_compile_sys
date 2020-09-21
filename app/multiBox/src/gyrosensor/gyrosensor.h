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

#ifndef __GYROSENSOR_H__
#define __GYROSENSOR_H__

#define MAX_NUM         200
#define GYRO_DIRECT_CALIBRATION 1
#define GYRO_SET_CALIBRATION    2

struct sensor_axis_data {
	int x;
	int y;
	int z;
};

struct sensor_data {
	long long timestamp_us;
	short temperature;
	long quaternion[4];
	struct sensor_axis_data gyro_axis;
	struct sensor_axis_data accel_axis;
	int quat_flag;
};

struct sensor_data_set {
	struct sensor_data data[MAX_NUM];
	int count;
};

#ifdef __cplusplus
extern "C" {
#endif

int gyrosensor_init(void);
void gyrosensor_deinit(void);
int gyrosensor_calibration(int status);
int gyrosensor_direct_getdata(struct sensor_data_set *data);
int gyrosensor_start(void);
int gyrosensor_stop(void);

#ifdef __cplusplus
}
#endif

#endif

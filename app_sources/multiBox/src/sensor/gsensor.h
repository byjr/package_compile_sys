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

#ifndef __GSENSOR_H__
#define __GSENSOR_H__

#define GSENSOR_INT_START       1
#define GSENSOR_INT_STOP        0
#define GSENSOR_INT1            1
#define GSENSOR_INT2            2

struct sensor_axis {
	float x;
	float y;
	float z;
};

typedef int fpI2C_Event(struct sensor_axis, int state);

int gsensor_init(void);
int gsensor_enable(int enable);
void gsensor_release(void);
int gsensor_regsenddatafunc(int (*send)(struct sensor_axis data));
int gsensor_unregsenddatafunc(int num);
int gsensor_use_interrupt(int intr_num, int intr_st);

int I2C_GSensor_Read(int addr, unsigned char *pdata);
int I2C_GSensor_Write(int addr, unsigned char *pdata);
int I2C_RegEventCallback(fpI2C_Event *EventCallBack);

#endif

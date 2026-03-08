/*
 * drivers/amlogic/media/dtv_demod/i2c_func.c
 *
 * Copyright (C) 2017 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/dvb/aml_demod.h>
#include "demod_func.h"

int aml_demod_i2c_read(struct i2c_adapter *i2c_adap, unsigned char slave_addr,
		unsigned char *data, unsigned short size)
{
	int ret = 0;
	struct i2c_msg msg;

	memset(&msg, 0, sizeof(msg));

	if (!i2c_adap) {
		PR_ERR("%s: i2c adapter NULL error.\n", __func__);

		return -EFAULT;
	}

	msg.addr = (slave_addr & 0x80) ? slave_addr >> 1 : slave_addr;
	msg.flags = I2C_M_RD;
	msg.len = size;
	msg.buf = data;

	ret = i2c_transfer(i2c_adap, &msg, 1);
	if (ret != 1) {
		PR_ERR("%s: error read (slave addr: 0x%x, ret: %d).\n",
			__func__, slave_addr, ret);

		return ret;
	}

	return 0;
}

int aml_demod_i2c_write(struct i2c_adapter *i2c_adap, unsigned char slave_addr,
		unsigned char *data, unsigned short size)
{
	int ret = 0;
	struct i2c_msg msg;

	memset(&msg, 0, sizeof(msg));

	if (!i2c_adap) {
		PR_ERR("%s: i2c adapter NULL error.\n", __func__);

		return -EFAULT;
	}

	msg.addr = (slave_addr & 0x80) ? slave_addr >> 1 : slave_addr;
	msg.flags = 0;
	msg.len = size;
	msg.buf = data;

	ret = i2c_transfer(i2c_adap, &msg, 1);
	if (ret != 1) {
		PR_ERR("%s: error write (slave addr: 0x%x, ret: %d).\n",
			__func__, slave_addr, ret);

		return ret;
	}

	return 0;
}

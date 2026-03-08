/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2021 Amlogic, Inc. All rights reserved.
 */

#ifndef __TPS65235_H__
#define __TPS65235_H__

#include "lnb_controller.h"

#define TPS65235_CONFIG_VOLTAGE_LOW_11_0V     0
#define TPS65235_CONFIG_VOLTAGE_LOW_11_6V     1
#define TPS65235_CONFIG_VOLTAGE_LOW_12_2V     2
#define TPS65235_CONFIG_VOLTAGE_LOW_12_8V     3
#define TPS65235_CONFIG_VOLTAGE_LOW_13_4V     4
#define TPS65235_CONFIG_VOLTAGE_LOW_14_0V     5
#define TPS65235_CONFIG_VOLTAGE_LOW_14_6V     6
#define TPS65235_CONFIG_VOLTAGE_LOW_15_2V     7
#define TPS65235_CONFIG_VOLTAGE_LOW_15_8V     8
#define TPS65235_CONFIG_VOLTAGE_HIGH_16_4V    9
#define TPS65235_CONFIG_VOLTAGE_HIGH_17_0V    10
#define TPS65235_CONFIG_VOLTAGE_HIGH_17_6V    11
#define TPS65235_CONFIG_VOLTAGE_HIGH_18_2V    12
#define TPS65235_CONFIG_VOLTAGE_HIGH_18_8V    13
#define TPS65235_CONFIG_VOLTAGE_HIGH_19_4V    14
#define TPS65235_CONFIG_VOLTAGE_HIGH_21_0V    15

extern int lnb_high_voltage;

int tps65235_create(struct lnbc *lnbc, struct i2c_adapter *i2c_adap,
		unsigned char i2c_addr);

#endif /* __TPS65235_H__ */


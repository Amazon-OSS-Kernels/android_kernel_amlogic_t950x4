/*
 * drivers/amlogic/media/dtv_demod/lnb_controller/gpio/gpio_lnbc.c
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

#include <linux/err.h>
#include "lnb_controller.h"
#include "gpio_lnbc.h"

static int gpio_lnbc_init(struct lnbc *lnbc);
static int gpio_lnbc_set_config(struct lnbc *lnbc,
		enum LNBC_CONFIG_ID config_id, int value);
static int gpio_lnbc_set_voltage(struct lnbc *lnbc, enum LNBC_VOLTAGE voltage);
static int gpio_lnbc_set_tone(struct lnbc *lnbc, enum LNBC_TONE tone);
static int gpio_lnbc_set_transmit_mode(struct lnbc *lnbc,
		enum LNBC_TRANSMIT_MODE transmit_mode);
static int gpio_lnbc_sleep(struct lnbc *lnbc);
static int gpio_lnbc_wake_up(struct lnbc *lnbc);
static int gpio_lnbc_get_diag_status(struct lnbc *lnbc, unsigned int *status,
		unsigned int *status_supported);

int gpio_lnbc_create(struct lnbc *lnbc, struct gpio_desc *gpio_lnb_en,
		struct gpio_desc *gpio_lnb_sel)
{
	if (!lnbc || !gpio_lnb_en || !gpio_lnb_sel)
		return -EFAULT;

	lnbc->init = gpio_lnbc_init;
	lnbc->set_config = gpio_lnbc_set_config;
	lnbc->set_voltage = gpio_lnbc_set_voltage;
	lnbc->set_tone = gpio_lnbc_set_tone;
	lnbc->set_transmit_mode = gpio_lnbc_set_transmit_mode;
	lnbc->sleep = gpio_lnbc_sleep;
	lnbc->wake_up = gpio_lnbc_wake_up;
	lnbc->get_diag_status = gpio_lnbc_get_diag_status;
	lnbc->is_internal_tone = 0;

	lnbc->gpio_lnb_en = gpio_lnb_en;
	lnbc->gpio_lnb_sel = gpio_lnb_sel;

	lnbc->state = LNBC_STATE_UNKNOWN;
	lnbc->voltage = LNBC_VOLTAGE_LOW;
	lnbc->tone = LNBC_TONE_AUTO;
	lnbc->transmit_mode = LNBC_TRANSMIT_MODE_TX;

	return 0;
}

static int gpio_lnbc_init(struct lnbc *lnbc)
{
	gpiod_set_value(lnbc->gpio_lnb_en, 0);
	gpiod_set_value(lnbc->gpio_lnb_sel, 0);

	lnbc->voltage = LNBC_VOLTAGE_OFF;
	lnbc->state = LNBC_STATE_SLEEP;

	return 0;
}

static int gpio_lnbc_set_config(struct lnbc *lnbc,
		enum LNBC_CONFIG_ID config_id, int value)
{
	return 0;
}

static int gpio_lnbc_set_voltage(struct lnbc *lnbc,
		enum LNBC_VOLTAGE voltage)
{
	if (lnbc->state != LNBC_STATE_ACTIVE && lnbc->state != LNBC_STATE_SLEEP)
		return -EINVAL;

	switch (voltage) {
	case LNBC_VOLTAGE_OFF:
		gpiod_set_value(lnbc->gpio_lnb_en, 0);
		gpiod_set_value(lnbc->gpio_lnb_sel, 0);
		break;

	case LNBC_VOLTAGE_LOW:
		gpiod_set_value(lnbc->gpio_lnb_en, 1);
		gpiod_set_value(lnbc->gpio_lnb_sel, 0);
		break;

	case LNBC_VOLTAGE_HIGH:
		gpiod_set_value(lnbc->gpio_lnb_en, 1);
		gpiod_set_value(lnbc->gpio_lnb_sel, 1);
		break;

	default:
		return -EINVAL;
	}

	lnbc->voltage = voltage;
	lnbc->state = LNBC_STATE_ACTIVE;

	return 0;
}

static int gpio_lnbc_set_tone(struct lnbc *lnbc, enum LNBC_TONE tone)
{
	return 0;
}

static int gpio_lnbc_set_transmit_mode(struct lnbc *lnbc,
		enum LNBC_TRANSMIT_MODE transmit_mode)
{
	return 0;
}

static int gpio_lnbc_sleep(struct lnbc *lnbc)
{
	if (!lnbc)
		return -EFAULT;

	gpiod_set_value(lnbc->gpio_lnb_en, 0);
	gpiod_set_value(lnbc->gpio_lnb_sel, 0);

	lnbc->voltage = LNBC_VOLTAGE_OFF;
	lnbc->state = LNBC_STATE_SLEEP;

	return 0;
}

static int gpio_lnbc_wake_up(struct lnbc *lnbc)
{
	if (!lnbc)
		return -EFAULT;

	lnbc->state = LNBC_STATE_ACTIVE;

	return 0;
}

static int gpio_lnbc_get_diag_status(struct lnbc *lnbc, unsigned int *status,
		unsigned int *status_supported)
{
	return 0;
}

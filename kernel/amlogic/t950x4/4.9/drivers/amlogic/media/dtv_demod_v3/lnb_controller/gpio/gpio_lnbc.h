/*
 * drivers/amlogic/media/dtv_demod/lnb_controller/gpio/gpio_lnbc.h
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
#ifndef __GPIO_LNBC_H__
#define __GPIO_LNBC_H__

#include "lnb_controller.h"

int gpio_lnbc_create(struct lnbc *lnbc, struct gpio_desc *gpio_lnb_en,
		struct gpio_desc *gpio_lnb_sel);

#endif /* __GPIO_LNBC_H__ */

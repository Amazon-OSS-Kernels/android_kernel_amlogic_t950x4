// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2021 Amlogic, Inc. All rights reserved.
 */

#include <linux/err.h>
#include "lnb_controller.h"
#include "tps65235.h"
#include "i2c_func.h"

static int create_1st_byte(struct lnbc *lnbc, enum LNBC_VOLTAGE voltage,
		unsigned char *data);
static int create_2nd_byte(struct lnbc *lnbc, unsigned char is_enable,
		enum LNBC_TRANSMIT_MODE mode, unsigned char *data);
static int tps65235_init(struct lnbc *lnbc);
static int tps65235_set_config(struct lnbc *lnbc,
		enum LNBC_CONFIG_ID config_id, int value);
static int tps65235_set_voltage(struct lnbc *lnbc,
		enum LNBC_VOLTAGE voltage);
static int tps65235_set_tone(struct lnbc *lnbc, enum LNBC_TONE tone);
static int tps65235_set_transmit_mode(struct lnbc *lnbc,
		enum LNBC_TRANSMIT_MODE transmit_mode);
static int tps65235_sleep(struct lnbc *lnbc);
static int tps65235_wake_up(struct lnbc *lnbc);
static int tps65235_get_diag_status(struct lnbc *lnbc, unsigned int *status,
		unsigned int *status_supported);

int tps65235_create(struct lnbc *lnbc, struct i2c_adapter *i2c_adap,
		unsigned char i2c_addr)
{
	if (!lnbc || !i2c_adap || !i2c_addr)
		return -EFAULT;

	lnbc->i2c_adap = i2c_adap;
	lnbc->i2c_addr = i2c_addr;
	lnbc->init = tps65235_init;
	lnbc->set_config = tps65235_set_config;
	lnbc->set_voltage = tps65235_set_voltage;
	lnbc->set_tone = tps65235_set_tone;
	lnbc->set_transmit_mode = tps65235_set_transmit_mode;
	lnbc->sleep = tps65235_sleep;
	lnbc->wake_up = tps65235_wake_up;
	lnbc->get_diag_status = tps65235_get_diag_status;
	lnbc->is_internal_tone = 0;

	if (lnb_high_voltage == 3) {
		lnbc->low_voltage = TPS65235_CONFIG_VOLTAGE_LOW_15_8V;
		lnbc->high_voltage = TPS65235_CONFIG_VOLTAGE_HIGH_21_0V;
	} else if (lnb_high_voltage == 2) {
		lnbc->low_voltage = TPS65235_CONFIG_VOLTAGE_LOW_15_2V;
		lnbc->high_voltage = TPS65235_CONFIG_VOLTAGE_HIGH_19_4V;
	} else if (lnb_high_voltage == 1) {
		lnbc->low_voltage = TPS65235_CONFIG_VOLTAGE_LOW_14_6V;
		lnbc->high_voltage = TPS65235_CONFIG_VOLTAGE_HIGH_18_8V;
	} else {
		/* Default value */
		lnbc->low_voltage = TPS65235_CONFIG_VOLTAGE_LOW_13_4V;
		lnbc->high_voltage = TPS65235_CONFIG_VOLTAGE_HIGH_18_2V;
	}

	lnbc->state = LNBC_STATE_UNKNOWN;
	lnbc->voltage = LNBC_VOLTAGE_LOW;
	lnbc->tone = LNBC_TONE_AUTO;
	lnbc->transmit_mode = LNBC_TRANSMIT_MODE_TX;

	return 0;
}

static int tps65235_init(struct lnbc *lnbc)
{
	int ret = 0;
	unsigned char data = 0;
	unsigned short len = 0;
	unsigned char buffer[2] = { 0 };

	if (!lnbc)
		return -EFAULT;

	lnbc->state = LNBC_STATE_UNKNOWN;
	lnbc->voltage = LNBC_VOLTAGE_OFF;
	lnbc->tone = LNBC_TONE_AUTO; /* Auto is only supported */

	if (lnbc->transmit_mode != LNBC_TRANSMIT_MODE_AUTO)
		lnbc->transmit_mode = LNBC_TRANSMIT_MODE_TX;

	ret = create_1st_byte(lnbc, lnbc->voltage, &data);
	if (ret)
		return ret;
	buffer[0] = 0x00;
	buffer[1] = data;
	len = 2;

	ret = aml_demod_i2c_write(lnbc->i2c_adap, lnbc->i2c_addr, buffer, len);
	if (ret)
		return ret;

	ret = create_2nd_byte(lnbc, 0, lnbc->transmit_mode, &data);
	if (ret)
		return ret;

	buffer[0] = 0x01;
	buffer[1] = data;
	len = 2;

	ret = aml_demod_i2c_write(lnbc->i2c_adap, lnbc->i2c_addr, buffer, len);
	if (ret)
		return ret;

	lnbc->state = LNBC_STATE_SLEEP;

	return 0;
}

static int tps65235_set_config(struct lnbc *lnbc,
		enum LNBC_CONFIG_ID config_id, int value)
{
	int ret = 0;

	if (!lnbc)
		return -EFAULT;

	if (lnbc->state != LNBC_STATE_ACTIVE &&
		lnbc->state != LNBC_STATE_SLEEP)
		return -EBUSY;

	switch (config_id) {
	case LNBC_CONFIG_ID_LOW_VOLTAGE:
		ret = tps65235_set_voltage(lnbc, lnbc->voltage);
		if (ret)
			return ret;

		lnbc->low_voltage = value;
		break;

	case LNBC_CONFIG_ID_HIGH_VOLTAGE:
		ret = tps65235_set_voltage(lnbc, lnbc->voltage);
		if (ret)
			return ret;

		lnbc->high_voltage = value;
		break;

	case LNBC_CONFIG_ID_TONE_INTERNAL:
		/* Internal/External tone is controlled by EXTM pin. No need to set register.
		 * External: gated by EXTM logic pulse, Internal: gated by EXTM logic envelop.
		 */
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int tps65235_set_voltage(struct lnbc *lnbc, enum LNBC_VOLTAGE voltage)
{
	int ret = 0;
	unsigned char data = 0;
	unsigned short len = 0;
	unsigned char buffer[2] = { 0 };

	if (!lnbc)
		return -EFAULT;

	if (lnbc->state != LNBC_STATE_ACTIVE &&
		lnbc->state != LNBC_STATE_SLEEP)
		return -EBUSY;

	if (voltage == LNBC_VOLTAGE_OFF) {
		ret = create_2nd_byte(lnbc, 0, lnbc->transmit_mode, &data);
		if (ret)
			return ret;

		buffer[0] = 0x01;
		buffer[1] = data;
		len = 2;

		ret = aml_demod_i2c_write(lnbc->i2c_adap, lnbc->i2c_addr, buffer, len);
		if (ret)
			return ret;
	} else {
		ret = create_1st_byte(lnbc, voltage, &data);
		if (ret)
			return ret;

		buffer[0] = 0x00;
		buffer[1] = data;
		len = 2;

		ret = aml_demod_i2c_write(lnbc->i2c_adap, lnbc->i2c_addr, buffer, len);
		if (ret)
			return ret;

		ret = create_2nd_byte(lnbc, 1, lnbc->transmit_mode, &data);
		if (ret)
			return ret;

		buffer[0] = 0x01;
		buffer[1] = data;
		len = 2;

		ret = aml_demod_i2c_write(lnbc->i2c_adap, lnbc->i2c_addr, buffer, len);
		if (ret)
			return ret;
	}

	lnbc->voltage = voltage;
	lnbc->state = LNBC_STATE_ACTIVE;

	return 0;
}

static int tps65235_set_tone(struct lnbc *lnbc, enum LNBC_TONE tone)
{
	if (!lnbc)
		return -EFAULT;

	if (lnbc->state != LNBC_STATE_ACTIVE &&
		lnbc->state != LNBC_STATE_SLEEP)
		return -EBUSY;

	switch (tone) {
	case LNBC_TONE_OFF:
	case LNBC_TONE_ON:
		/* doesn't support this function. */
		break;

	case LNBC_TONE_AUTO:
		break;

	default:
		return -EFAULT;
	}

	lnbc->tone = tone;

	return 0;
}

static int tps65235_set_transmit_mode(struct lnbc *lnbc,
		enum LNBC_TRANSMIT_MODE transmit_mode)
{
	int ret = 0;
	unsigned char data = 0;
	unsigned short len = 0;
	unsigned char buffer[2] = { 0 };

	if (!lnbc)
		return -EFAULT;

	if (lnbc->state != LNBC_STATE_ACTIVE &&
		lnbc->state != LNBC_STATE_SLEEP)
		return -EBUSY;

	ret = create_2nd_byte(lnbc, 1, transmit_mode, &data);
	if (ret)
		return ret;

	buffer[0] = 0x01;
	buffer[1] = data;
	len = 2;
	ret = aml_demod_i2c_write(lnbc->i2c_adap, lnbc->i2c_addr, buffer, len);
	if (ret)
		return ret;

	lnbc->transmit_mode = transmit_mode;

	return 0;
}

static int tps65235_sleep(struct lnbc *lnbc)
{
	int ret = 0;
	unsigned char data = 0;
	unsigned short len = 0;
	unsigned char buffer[2] = { 0 };

	if (!lnbc)
		return -EFAULT;

	switch (lnbc->state) {
	case LNBC_STATE_ACTIVE:
		/* Continue */
		break;

	case LNBC_STATE_SLEEP:
		/* Do nothing */
		return 0;

	case LNBC_STATE_UNKNOWN:
	default:
		/* Error */
		return -EINVAL;
	}

	ret = create_2nd_byte(lnbc, 0, lnbc->transmit_mode, &data);
	if (ret)
		return ret;

	buffer[0] = 0x01;
	buffer[1] = data;
	len = 2;
	ret = aml_demod_i2c_write(lnbc->i2c_adap, lnbc->i2c_addr, buffer, len);
	if (ret)
		return ret;

	lnbc->state = LNBC_STATE_SLEEP;

	return 0;
}

static int tps65235_wake_up(struct lnbc *lnbc)
{
	int ret = 0;
	unsigned char data = 0;
	unsigned short len = 0;
	unsigned char buffer[2] = { 0 };

	if (!lnbc)
		return -EFAULT;

	switch (lnbc->state) {
	case LNBC_STATE_ACTIVE:
		/* Do nothing */
		return 0;

	case LNBC_STATE_SLEEP:
		/* Continue */
		break;

	case LNBC_STATE_UNKNOWN:
	default:
		/* Error */
		return -EINVAL;
	}

	ret = create_1st_byte(lnbc, lnbc->voltage, &data);
	if (ret)
		return ret;

	buffer[0] = 0x00;
	buffer[1] = data;
	len = 2;
	ret = aml_demod_i2c_write(lnbc->i2c_adap, lnbc->i2c_addr, buffer, len);
	if (ret)
		return ret;

	ret = create_2nd_byte(lnbc, 1, lnbc->transmit_mode, &data);
	if (ret)
		return ret;

	buffer[0] = 0x01;
	buffer[1] = data;
	len = 2;
	ret = aml_demod_i2c_write(lnbc->i2c_adap, lnbc->i2c_addr, buffer, len);
	if (ret)
		return ret;

	lnbc->state = LNBC_STATE_ACTIVE;

	return 0;
}

static int tps65235_get_diag_status(struct lnbc *lnbc, unsigned int *status,
		unsigned int *status_supported)
{
	int ret = 0;
	unsigned char data = 0;
	unsigned short len = 0;

	if (!lnbc || !status)
		return -EFAULT;

	if (status_supported) {
		*status_supported = LNBC_DIAG_STATUS_VOUTOUTOFRANGE |
		LNBC_DIAG_STATUS_CABLEOPEN |
		LNBC_DIAG_STATUS_OVERCURRENT |
		LNBC_DIAG_STATUS_THERMALSHUTDOWN;
	}

	data = 0x02;
	len = 1;
	ret = aml_demod_i2c_write(lnbc->i2c_adap, lnbc->i2c_addr, &data, len);
	if (ret)
		return ret;

	len = 1;
	ret = aml_demod_i2c_read(lnbc->i2c_adap, lnbc->i2c_addr, &data, len);
	if (ret)
		return ret;

	/* Status Register
	 * +---------+---------+---------+---------+---------+---------+----------+----------+
	 * | Reserve |TDETGOOD | LDO_ON  |  T125   |   TSD   |   OCP   |CABLE_GOOD|VOUT_GOOD |
	 * +---------+---------+---------+---------+---------+---------+----------+----------+
	 */

	*status = 0;

	if (!(data & 0x01))
		*status |= LNBC_DIAG_STATUS_VOUTOUTOFRANGE;

	if (!(data & 0x02))
		*status |= LNBC_DIAG_STATUS_CABLEOPEN;

	if (data & 0x04)
		*status |= LNBC_DIAG_STATUS_OVERCURRENT;

	if (data & 0x08)
		*status |= LNBC_DIAG_STATUS_THERMALSHUTDOWN;

	return 0;
}

static int create_1st_byte(struct lnbc *lnbc, enum LNBC_VOLTAGE voltage,
		unsigned char *data)
{
	if (!lnbc || !data)
		return -EFAULT;

	/* Control Register 1
	 * +-------+-------+-------+-------+-------+-------+-------+---------+
	 * |I2C_CON|PWM/PSM|   -   | VSET3 | VSET2 | VSET1 | VSET0 |EXTM_TONE|
	 * +-------+-------+-------+-------+-------+-------+-------+---------+
	 * |   ?   |   1   |   0   |   ?   |   ?   |   ?   |   ?   |    0    |
	 * +-------+-------+-------+-------+-------+-------+-------+---------+
	 * I2C_CON = 0 : I2C control disabled
	 *           1 : I2C control enabled
	 *
	 * +------+------+------+------+------+
	 * |VSET3 |VSET2 |VSET1 |VSET0 |LNB(V)|
	 * +------+------+------+------+------+
	 */
	*data = 0x40;

	switch (voltage) {
	case LNBC_VOLTAGE_OFF:
		break;

	case LNBC_VOLTAGE_LOW:
		*data |= 0x80; /* I2C_CON = 1 */
		switch (lnbc->low_voltage) {
		case TPS65235_CONFIG_VOLTAGE_LOW_11_0V:
		case TPS65235_CONFIG_VOLTAGE_LOW_11_6V:
		case TPS65235_CONFIG_VOLTAGE_LOW_12_2V:
		case TPS65235_CONFIG_VOLTAGE_LOW_12_8V:
		case TPS65235_CONFIG_VOLTAGE_LOW_13_4V:
		case TPS65235_CONFIG_VOLTAGE_LOW_14_0V:
		case TPS65235_CONFIG_VOLTAGE_LOW_14_6V:
		case TPS65235_CONFIG_VOLTAGE_LOW_15_2V:
		case TPS65235_CONFIG_VOLTAGE_LOW_15_8V:
			*data |= (lnbc->low_voltage << 1);
			break;

		default:
			return -EINVAL;
		}
		break;

	case LNBC_VOLTAGE_HIGH:
		*data |= 0x80; /* I2C_CON = 1 */
		switch (lnbc->high_voltage) {
		case TPS65235_CONFIG_VOLTAGE_HIGH_16_4V:
		case TPS65235_CONFIG_VOLTAGE_HIGH_17_0V:
		case TPS65235_CONFIG_VOLTAGE_HIGH_17_6V:
		case TPS65235_CONFIG_VOLTAGE_HIGH_18_2V:
		case TPS65235_CONFIG_VOLTAGE_HIGH_18_8V:
		case TPS65235_CONFIG_VOLTAGE_HIGH_19_4V:
		case TPS65235_CONFIG_VOLTAGE_HIGH_21_0V:
			*data |= (lnbc->high_voltage << 1);
			break;

		default:
			return -EINVAL;
		}
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int create_2nd_byte(struct lnbc *lnbc, unsigned char is_enable,
		enum LNBC_TRANSMIT_MODE mode, unsigned char *data)
{
	if (!lnbc || !data)
		return -EFAULT;

	/* Control Register 2
	 * +---------+---------+---------+---------+---------+---------+---------+----------+
	 * | TONEAMP |  TIMER  |  l_SW   |  FSET   |   EN    |DOUTMODE |TONE_AUTO|TONE_TRANS|
	 * +---------+---------+---------+---------+---------+---------+---------+----------+
	 * |    0    |    0    |    0    |    1    |    ?    |    0    |    ?    |    ?     |
	 * +---------+---------+---------+---------+---------+---------+---------+----------+
	 * EN         = 0 : LNB output disabled
	 *              1 : LNB output voltage Enabled
	 * TONE_AUTO  = 0 : GDR (External bypass FET control) is controlled by TONE_TRANS
	 *              1 : GDR (External bypass FET control) is automatically controlled
	 *                  by 22kHz tones transmit.
	 * TONE_TRANS = 0 : GDR output with VLNB voltage. Bypass FET is OFF for tone receiving
	 *                  from satellite.
	 *              1 : GDR output with VCP voltage. Bypass FET is ON for tone transmit
	 *                  from TPS65235.
	 */
	*data = 0x10;

	if (is_enable)
		*data |= 0x08;

	switch (mode) {
	case LNBC_TRANSMIT_MODE_TX:
		/* TONE_TRANS = 1 */
		*data |= 0x01;
		break;

	case LNBC_TRANSMIT_MODE_RX:
		/* TONE_TRANS = 0 */
		break;

	case LNBC_TRANSMIT_MODE_AUTO:
		/* TONE_AUTO = 1 */
		*data |= 0x02;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/*
 * drivers/amlogic/pinctrl/pinctrl-meson-t5.c
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

#include <dt-bindings/gpio/meson-t5-gpio.h>
#include <linux/arm-smccc.h>
#include "pinctrl-meson.h"
#include "pinctrl-meson-axg-pmx.h"
#include "pinconf-meson-g12a.h"

static const struct pinctrl_pin_desc meson_t5_analog_pins[] = {
	MESON_PIN(CDAC_IOUT),
	MESON_PIN(CVBS0),
	MESON_PIN(CVBS1)
};

static struct meson_pmx_group meson_t5_analog_groups[] = {
	GPIO_GROUP(CDAC_IOUT),
	GPIO_GROUP(CVBS0),
	GPIO_GROUP(CVBS1)
};

static const char * const gpio_analog_groups[] = {
	"CDAC_IOUT", "CVBS0", "CVBS1"
};

static struct meson_pmx_func meson_t5_analog_functions[] = {
	FUNCTION(gpio_analog)
};

static const struct pinctrl_pin_desc meson_t5_aobus_pins[] = {
	MESON_PIN(GPIOD_0),
	MESON_PIN(GPIOD_1),
	MESON_PIN(GPIOD_2),
	MESON_PIN(GPIOD_3),
	MESON_PIN(GPIOD_4),
	MESON_PIN(GPIOD_5),
	MESON_PIN(GPIOD_6),
	MESON_PIN(GPIOD_7),
	MESON_PIN(GPIOD_8),
	MESON_PIN(GPIOD_9),
	MESON_PIN(GPIOD_10),
	MESON_PIN(GPIOE_0),
	MESON_PIN(GPIOE_1),
	MESON_PIN(GPIOE_2)

};

/* D func 1*/
static const unsigned int uart_ao_b_tx_pins[] = { GPIOD_0 };
static const unsigned int uart_ao_b_rx_pins[] = { GPIOD_1 };
static const unsigned int i2c0_ao_sck_pins[] = { GPIOD_2 };
static const unsigned int i2c0_ao_sda_pins[] = { GPIOD_3 };
static const unsigned int clk_32k_in_pins[] = { GPIOD_4 };
static const unsigned int remote_input_ao_pins[] = { GPIOD_5 };
static const unsigned int jtag_a_clk_pins[] = { GPIOD_6 };
static const unsigned int jtag_a_tms_pins[] = { GPIOD_7 };
static const unsigned int jtag_a_tdi_pins[] = { GPIOD_8 };
static const unsigned int jtag_a_tdo_pins[] = { GPIOD_9 };
static const unsigned int gen_clk_ee_pins[] = { GPIOD_10};

/* D func 2 */
static const unsigned int remote_out_ao1_pins[] = { GPIOD_1 };
static const unsigned int i2c_ao_slave_sck_pins[] = {GPIOD_2};
static const unsigned int i2c_ao_slave_sda_pins[] = {GPIOD_3};
static const unsigned int atv_if_agc_ao_pins[] = { GPIOD_4 };
static const unsigned int pwm_ao_c_d5_pins[] = { GPIOD_5 };
static const unsigned int pwm_ao_d_d6_pins[] = { GPIOD_6 };
static const unsigned int pwm_ao_c_d7_pins[] = { GPIOD_7 };
static const unsigned int spdif_out_b_ao_pins[] = { GPIOD_8 };
static const unsigned int pwm_ao_d_d9_pins[] = { GPIOD_9 };
static const unsigned int pwm_ao_e_pins[] = { GPIOD_10 };

/* D func 3 */
static const unsigned int dtv_if_agc_ao_pins[] = { GPIOD_4 };
static const unsigned int pwm_ao_d_hiz_pins[] = { GPIOD_6 };
static const unsigned int pwm_ao_c_hiz_pins[] = { GPIOD_7 };
static const unsigned int remote_out_ao9_pins[] = { GPIOD_9 };
static const unsigned int gen_clk_ao_pins[] = { GPIOD_10 };

/* D func 4 */
static const unsigned int uart_ao_c_tx_pins[] = { GPIOD_6 };
static const unsigned int uart_ao_c_rx_pins[] = { GPIOD_7 };
static const unsigned int uart_ao_c_cts_pins[] = { GPIOD_8 };
static const unsigned int uart_ao_c_rts_pins[] = { GPIOD_9 };

/* E func 1 */
static const unsigned int pwm_ao_a_pins[] = { GPIOE_0 };
static const unsigned int pwm_ao_b_pins[] = { GPIOE_1 };
static const unsigned int spdif_out_a_ao_pins[] = { GPIOE_2 };

/* E func 2 */
static const unsigned int i2c2_ao_sck_pins[] = { GPIOE_0 };
static const unsigned int i2c2_ao_sda_pins[] = { GPIOE_1 };

static struct meson_pmx_group meson_t5_aobus_groups[] = {
	GPIO_GROUP(GPIOD_0),
	GPIO_GROUP(GPIOD_1),
	GPIO_GROUP(GPIOD_2),
	GPIO_GROUP(GPIOD_3),
	GPIO_GROUP(GPIOD_4),
	GPIO_GROUP(GPIOD_5),
	GPIO_GROUP(GPIOD_6),
	GPIO_GROUP(GPIOD_7),
	GPIO_GROUP(GPIOD_8),
	GPIO_GROUP(GPIOD_9),
	GPIO_GROUP(GPIOD_10),
	GPIO_GROUP(GPIOE_0),
	GPIO_GROUP(GPIOE_1),
	GPIO_GROUP(GPIOE_2),

	/* bank D */
	GROUP(uart_ao_b_tx,	1),
	GROUP(uart_ao_b_rx,	1),
	GROUP(i2c0_ao_sck,	1),
	GROUP(i2c0_ao_sda,	1),
	GROUP(clk_32k_in,	1),
	GROUP(remote_input_ao,	1),
	GROUP(jtag_a_clk,	1),
	GROUP(jtag_a_tms,	1),
	GROUP(jtag_a_tdi,	1),
	GROUP(jtag_a_tdo,	1),
	GROUP(gen_clk_ee,	1),

	GROUP(remote_out_ao1,	2),
	GROUP(i2c_ao_slave_sck,	2),
	GROUP(i2c_ao_slave_sda,	2),
	GROUP(atv_if_agc_ao,	2),
	GROUP(pwm_ao_c_d5,	2),
	GROUP(pwm_ao_d_d6,	2),
	GROUP(pwm_ao_c_d7,	2),
	GROUP(spdif_out_b_ao,	2),
	GROUP(pwm_ao_d_d9,	2),
	GROUP(pwm_ao_e,		2),

	GROUP(dtv_if_agc_ao,	3),
	GROUP(pwm_ao_d_hiz,	3),
	GROUP(pwm_ao_c_hiz,	3),
	GROUP(remote_out_ao9,	3),
	GROUP(gen_clk_ao,	3),

	GROUP(uart_ao_c_tx,	4),
	GROUP(uart_ao_c_rx,	4),
	GROUP(uart_ao_c_cts,	4),
	GROUP(uart_ao_c_rts,	4),

	/* bank E */
	GROUP(pwm_ao_a,		1),
	GROUP(pwm_ao_b,		1),
	GROUP(spdif_out_a_ao,	1),

	GROUP(i2c2_ao_sck,	2),
	GROUP(i2c2_ao_sda,	2)

};

static const char * const gpio_aobus_groups[] = {
	"GPIOD_0", "GPIOD_1", "GPIOD_2", "GPIOD_3",
	"GPIOD_4", "GPIOD_5", "GPIOD_6", "GPIOD_7",
	"GPIOD_8", "GPIOD_9", "GPIOD_10", "GPIOE_0",
	"GPIOE_1", "GPIOE_2"
};

static const char * const uart_ao_b_groups[] = {
	"uart_ao_b_tx", "uart_ao_b_rx"
};

static const char * const uart_ao_c_groups[] = {
	"uart_ao_c_tx", "uart_ao_c_rx", "uart_ao_c_cts", "uart_ao_c_rts"
};

static const char * const i2c0_ao_groups[] = {
	"i2c0_ao_sck", "i2c0_ao_sda"
};

static const char * const i2c2_ao_groups[] = {
	"i2c2_ao_sck", "i2c2_ao_sda"
};

static const char * const clk_32k_in_groups[] = {
	"clk_32k_in"
};

static const char * const remote_input_ao_groups[] = {
	"remote_input_ao"
};

static const char * const jtag_a_groups[] = {
	"jtag_a_clk", "jtag_a_tms", "jtag_a_tdi", "jtag_a_tdo"
};

static const char * const gen_clk_ee_groups[] = {
	"gen_clk_ee"
};

static const char * const remote_out_ao_groups[] = {
	"remote_out_ao1", "remote_out_ao9"
};

static const char * const pwm_ao_a_groups[] = {
	"pwm_ao_a"
};

static const char * const pwm_ao_b_groups[] = {
	"pwm_ao_b"
};

static const char * const pwm_ao_c_groups[] = {
	"pwm_ao_c_d7", "pwm_ao_c_d5"
};

static const char * const pwm_ao_d_groups[] = {
	"pwm_ao_d_d6", "pwm_ao_d_d9"
};

static const char * const pwm_ao_e_groups[] = {
	"pwm_ao_e"
};

static const char * const pwm_ao_c_hiz_groups[] = {
	"pwm_ao_c_hiz"
};

static const char * const pwm_ao_d_hiz_groups[] = {
	"pwm_ao_d_hiz"
};

static const char * const spdif_out_a_ao_groups[] = {
	"spdif_out_a_ao"
};

static const char * const spdif_out_b_ao_groups[] = {
	"spdif_out_b_ao"
};

static const char * const dtv_ao_groups[] = {
	"dtv_if_agc_ao"
};

static const char * const atv_ao_groups[] = {
	"atv_if_agc_ao"
};

static const char * const gen_clk_ao_groups[] = {
	"gen_clk_ao"
};

static struct meson_pmx_func meson_t5_aobus_functions[] = {
	FUNCTION(gpio_aobus),
	FUNCTION(uart_ao_b),
	FUNCTION(uart_ao_c),
	FUNCTION(i2c0_ao),
	FUNCTION(i2c2_ao),
	FUNCTION(clk_32k_in),
	FUNCTION(remote_input_ao),
	FUNCTION(remote_out_ao),
	FUNCTION(jtag_a),
	FUNCTION(gen_clk_ee),
	FUNCTION(gen_clk_ao),
	FUNCTION(pwm_ao_a),
	FUNCTION(pwm_ao_b),
	FUNCTION(pwm_ao_c),
	FUNCTION(pwm_ao_d),
	FUNCTION(pwm_ao_e),
	FUNCTION(pwm_ao_c_hiz),
	FUNCTION(pwm_ao_d_hiz),
	FUNCTION(spdif_out_a_ao),
	FUNCTION(spdif_out_b_ao),
	FUNCTION(dtv_ao),
	FUNCTION(atv_ao)
};

static const struct pinctrl_pin_desc meson_t5_periphs_pins[] = {
	MESON_PIN(GPIOB_0),
	MESON_PIN(GPIOB_1),
	MESON_PIN(GPIOB_2),
	MESON_PIN(GPIOB_3),
	MESON_PIN(GPIOB_4),
	MESON_PIN(GPIOB_5),
	MESON_PIN(GPIOB_6),
	MESON_PIN(GPIOB_7),
	MESON_PIN(GPIOB_8),
	MESON_PIN(GPIOB_9),
	MESON_PIN(GPIOB_10),
	MESON_PIN(GPIOB_11),
	MESON_PIN(GPIOB_12),
	MESON_PIN(GPIOB_13),
	MESON_PIN(GPIOW_0),
	MESON_PIN(GPIOW_1),
	MESON_PIN(GPIOW_2),
	MESON_PIN(GPIOW_3),
	MESON_PIN(GPIOW_4),
	MESON_PIN(GPIOW_5),
	MESON_PIN(GPIOW_6),
	MESON_PIN(GPIOW_7),
	MESON_PIN(GPIOW_8),
	MESON_PIN(GPIOW_9),
	MESON_PIN(GPIOW_10),
	MESON_PIN(GPIOW_11),
	MESON_PIN(GPIOW_12),
	MESON_PIN(GPIOZ_0),
	MESON_PIN(GPIOZ_1),
	MESON_PIN(GPIOZ_2),
	MESON_PIN(GPIOZ_3),
	MESON_PIN(GPIOZ_4),
	MESON_PIN(GPIOZ_5),
	MESON_PIN(GPIOZ_6),
	MESON_PIN(GPIOH_0),
	MESON_PIN(GPIOH_1),
	MESON_PIN(GPIOH_2),
	MESON_PIN(GPIOH_3),
	MESON_PIN(GPIOH_4),
	MESON_PIN(GPIOH_5),
	MESON_PIN(GPIOH_6),
	MESON_PIN(GPIOH_7),
	MESON_PIN(GPIOH_8),
	MESON_PIN(GPIOH_9),
	MESON_PIN(GPIOH_10),
	MESON_PIN(GPIOH_11),
	MESON_PIN(GPIOH_12),
	MESON_PIN(GPIOH_13),
	MESON_PIN(GPIOH_14),
	MESON_PIN(GPIOH_15),
	MESON_PIN(GPIOH_16),
	MESON_PIN(GPIOH_17),
	MESON_PIN(GPIOH_18),
	MESON_PIN(GPIOH_19),
	MESON_PIN(GPIOH_20),
	MESON_PIN(GPIOH_21)
};

/* emmc */
static const unsigned int emmc_d0_pins[] = {GPIOB_0};
static const unsigned int emmc_d1_pins[] = {GPIOB_1};
static const unsigned int emmc_d2_pins[] = {GPIOB_2};
static const unsigned int emmc_d3_pins[] = {GPIOB_3};
static const unsigned int emmc_d4_pins[] = {GPIOB_4};
static const unsigned int emmc_d5_pins[] = {GPIOB_5};
static const unsigned int emmc_d6_pins[] = {GPIOB_6};
static const unsigned int emmc_d7_pins[] = {GPIOB_7};
static const unsigned int emmc_clk_pins[] = {GPIOB_8};
static const unsigned int emmc_rst_pins[] = {GPIOB_9};
static const unsigned int emmc_cmd_pins[] = {GPIOB_10};
static const unsigned int emmc_ds_pins[] = {GPIOB_11};

/* spif */
static const unsigned int spif_hold_pins[] = {GPIOB_3};
static const unsigned int spif_mo_pins[] = {GPIOB_4};
static const unsigned int spif_mi_pins[] = {GPIOB_5};
static const unsigned int spif_clk_pins[] = {GPIOB_6};
static const unsigned int spif_wp_pins[] = {GPIOB_7};
static const unsigned int spif_cs_pins[] = {GPIOB_13};

/*loop ? */
static const unsigned int loop_gpio_b12_pins[] = {GPIOB_13};
static const unsigned int loop_gpio_h21_pins[] = {GPIOB_13};
static const unsigned int loop_gpio_b13_pins[] = {GPIOH_21};

/*ir out*/
static const unsigned int remote_out_pins[] = {GPIOB_12};

/* hdmirx */
static const unsigned int hdmirx_a_hpd_pins[] = {GPIOW_0};
static const unsigned int hdmirx_a_det_pins[] = {GPIOW_1};
static const unsigned int hdmirx_a_sda_pins[] = {GPIOW_2};
static const unsigned int hdmirx_a_sck_pins[] = {GPIOW_3};
static const unsigned int hdmirx_c_hpd_pins[] = {GPIOW_4};
static const unsigned int hdmirx_c_det_pins[] = {GPIOW_5};
static const unsigned int hdmirx_c_sda_pins[] = {GPIOW_6};
static const unsigned int hdmirx_c_sck_pins[] = {GPIOW_7};
static const unsigned int hdmirx_b_hpd_pins[] = {GPIOW_8};
static const unsigned int hdmirx_b_det_pins[] = {GPIOW_9};
static const unsigned int hdmirx_b_sda_pins[] = {GPIOW_10};
static const unsigned int hdmirx_b_sck_pins[] = {GPIOW_11};

/* cec */
static const unsigned int cec_pins[] = {GPIOW_12};

/* uart_a */
static const unsigned int uart_a_tx_pins[] = {GPIOZ_0};
static const unsigned int uart_a_rx_pins[] = {GPIOZ_1};
static const unsigned int uart_a_cts_pins[] = {GPIOZ_2};
static const unsigned int uart_a_rts_pins[] = {GPIOZ_3};

/* uart_b */
static const unsigned int uart_b_tx_w2_pins[] = {GPIOW_2};
static const unsigned int uart_b_rx_w3_pins[] = {GPIOW_3};
static const unsigned int uart_b_tx_w6_pins[] = {GPIOW_6};
static const unsigned int uart_b_rx_w7_pins[] = {GPIOW_7};
static const unsigned int uart_b_tx_w10_pins[] = {GPIOW_10};
static const unsigned int uart_b_rx_w11_pins[] = {GPIOW_11};

/* uart_c */
static const unsigned int uart_c_tx_h_pins[] = {GPIOH_1};
static const unsigned int uart_c_rx_h_pins[] = {GPIOH_2};
static const unsigned int uart_c_cts_h_pins[] = {GPIOH_3};
static const unsigned int uart_c_rts_h_pins[] = {GPIOH_4};

/* tdm */
static const unsigned int tdm_fs2_pins[] = {GPIOZ_0};
static const unsigned int tdm_sclk2_pins[] = {GPIOZ_1};
static const unsigned int tdm_d4_pins[] = {GPIOZ_2};
static const unsigned int tdm_d5_pins[] = {GPIOZ_3};
static const unsigned int tdm_d6_pins[] = {GPIOZ_4};
static const unsigned int tdm_d7_pins[] = {GPIOZ_5};
static const unsigned int tdm_d3_pins[] = {GPIOH_6};
static const unsigned int tdm_d3_h6_pins[] = {GPIOH_6};
static const unsigned int tdm_d3_h13_pins[] = {GPIOH_13};
static const unsigned int tdm_sclk1_pins[] = {GPIOH_15};
static const unsigned int tdm_fs1_pins[] = {GPIOH_16};
static const unsigned int tdm_d0_pins[] = {GPIOH_17};
static const unsigned int tdm_d1_pins[] = {GPIOH_18};
static const unsigned int tdm_d2_pins[] = {GPIOH_19};

/* mclk */
static const unsigned int mclk_1_pins[] = {GPIOH_14};
static const unsigned int mclk_2_pins[] = {GPIOZ_6};

/* tsin_a */
static const unsigned int tsin_a_clk_pins[] = {GPIOZ_0};
static const unsigned int tsin_a_sop_pins[] = {GPIOZ_1};
static const unsigned int tsin_a_valid_pins[] = {GPIOZ_2};
static const unsigned int tsin_a_d0_pins[] = {GPIOZ_3};

/* i2c_0 */
static const unsigned int i2c0_sck_z_pins[] = {GPIOZ_4};
static const unsigned int i2c0_sda_z_pins[] = {GPIOZ_5};

/* i2c_1 */
static const unsigned int i2c1_sck_h_pins[] = {GPIOH_20};
static const unsigned int i2c1_sda_h_pins[] = {GPIOH_21};

/* i2c_2 */
static const unsigned int i2c2_sck_h_pins[] = {GPIOH_10};
static const unsigned int i2c2_sda_h_pins[] = {GPIOH_11};

/* atv_if_agc */
static const unsigned int atv_if_agc_pins[] = {GPIOZ_6};

/* spi */
static const unsigned int spi0_miso_z_pins[] = {GPIOZ_0};
static const unsigned int spi0_mosi_z_pins[] = {GPIOZ_1};
static const unsigned int spi0_clk_z_pins[] =  {GPIOZ_2};
static const unsigned int spi0_ss0_z_pins[] = {GPIOZ_3};
static const unsigned int spi0_ss1_z_pins[] = {GPIOZ_4};
static const unsigned int spi0_ss2_z_pins[] = {GPIOZ_5};

static const unsigned int spi0_ss0_h_pins[] = {GPIOH_7};
static const unsigned int spi0_ss1_h_pins[] = {GPIOH_8};
static const unsigned int spi0_miso_h_pins[] = {GPIOH_9};
static const unsigned int spi0_mosi_h_pins[] = {GPIOH_10};
static const unsigned int spi0_clk_h_pins[] =  {GPIOH_11};
static const unsigned int spi0_ss2_h_pins[] = {GPIOH_12};

/* dtv_if_agc */
static const unsigned int dtv_if_agc_pins[] = {GPIOZ_6};

/* pwm_b */
static const unsigned int pwm_b_z_pins[] = {GPIOZ_4};
static const unsigned int pwm_b_h_pins[] = {GPIOH_14};

/* pwm_d */
static const unsigned int pwm_d_pins[] = {GPIOH_5};

/* pwm_f */
static const unsigned int pwm_f_z_pins[] = {GPIOZ_5};
static const unsigned int pwm_f_h_pins[] = {GPIOH_13};

/* pwm_e */
static const unsigned int pwm_e_z_pins[] = {GPIOZ_6};
static const unsigned int pwm_e_h_pins[] = {GPIOH_12};

/* pwm_vs */
static const unsigned int pwm_vs_h12_pins[] = {GPIOH_12};
static const unsigned int pwm_vs_h13_pins[] = {GPIOH_13};

/* pdm */
static const unsigned int pdm_din1_z1_pins[] = {GPIOZ_1};
static const unsigned int pdm_dclk_z_pins[] = {GPIOZ_2};
static const unsigned int pdm_din0_z_pins[] = {GPIOZ_3};
static const unsigned int pdm_din1_z6_pins[] = {GPIOZ_6};
static const unsigned int pdm_din1_h14_pins[] = {GPIOH_14};
static const unsigned int pdm_din0_h18_pins[] = {GPIOH_18};
static const unsigned int pdm_dclk_h_pins[] = {GPIOH_19};

/* tcon */
static const unsigned int tcon_0_pins[] = {GPIOH_0};
static const unsigned int tcon_1_pins[] = {GPIOH_1};
static const unsigned int tcon_2_pins[] = {GPIOH_2};
static const unsigned int tcon_3_pins[] = {GPIOH_3};
static const unsigned int tcon_4_pins[] = {GPIOH_4};
static const unsigned int tcon_5_pins[] = {GPIOH_5};
static const unsigned int tcon_6_pins[] = {GPIOH_6};
static const unsigned int tcon_7_pins[] = {GPIOH_7};
static const unsigned int tcon_8_pins[] = {GPIOH_8};
static const unsigned int tcon_9_pins[] = {GPIOH_9};
static const unsigned int tcon_10_pins[] = {GPIOH_10};
static const unsigned int tcon_11_pins[] = {GPIOH_11};
static const unsigned int tcon_12_pins[] = {GPIOH_12};
static const unsigned int tcon_13_pins[] = {GPIOH_13};
static const unsigned int tcon_14_pins[] = {GPIOH_14};
static const unsigned int tcon_15_pins[] = {GPIOH_15};
static const unsigned int tcon_lock_pins[] = {GPIOH_0};
static const unsigned int tcon_sfc_h0_pins[] = {GPIOH_0};
static const unsigned int tcon_sfc_h8_pins[] = {GPIOH_8};

/* sync */
static const unsigned int hsync_pins[] = {GPIOH_18};
static const unsigned int vsync_pins[] = {GPIOH_19};
static const unsigned int sync_3d_out_pins[] = {GPIOH_5};

/* vx1 */
static const unsigned int vx1_lockn_pins[] = {GPIOH_0};
static const unsigned int vx1_htpdn_pins[] = {GPIOH_8};

/* eth */
static const unsigned int eth_act_led_pins[] = {GPIOH_18};
static const unsigned int eth_link_led_pins[] = {GPIOH_19};

static struct meson_pmx_group meson_t5_periphs_groups[] = {
	GPIO_GROUP(GPIOB_0),
	GPIO_GROUP(GPIOB_1),
	GPIO_GROUP(GPIOB_2),
	GPIO_GROUP(GPIOB_3),
	GPIO_GROUP(GPIOB_4),
	GPIO_GROUP(GPIOB_5),
	GPIO_GROUP(GPIOB_6),
	GPIO_GROUP(GPIOB_7),
	GPIO_GROUP(GPIOB_8),
	GPIO_GROUP(GPIOB_9),
	GPIO_GROUP(GPIOB_10),
	GPIO_GROUP(GPIOB_11),
	GPIO_GROUP(GPIOB_12),
	GPIO_GROUP(GPIOB_13),
	GPIO_GROUP(GPIOW_0),
	GPIO_GROUP(GPIOW_1),
	GPIO_GROUP(GPIOW_2),
	GPIO_GROUP(GPIOW_3),
	GPIO_GROUP(GPIOW_4),
	GPIO_GROUP(GPIOW_5),
	GPIO_GROUP(GPIOW_6),
	GPIO_GROUP(GPIOW_7),
	GPIO_GROUP(GPIOW_8),
	GPIO_GROUP(GPIOW_9),
	GPIO_GROUP(GPIOW_10),
	GPIO_GROUP(GPIOW_11),
	GPIO_GROUP(GPIOW_12),
	GPIO_GROUP(GPIOZ_0),
	GPIO_GROUP(GPIOZ_1),
	GPIO_GROUP(GPIOZ_2),
	GPIO_GROUP(GPIOZ_3),
	GPIO_GROUP(GPIOZ_4),
	GPIO_GROUP(GPIOZ_5),
	GPIO_GROUP(GPIOZ_6),
	GPIO_GROUP(GPIOH_0),
	GPIO_GROUP(GPIOH_1),
	GPIO_GROUP(GPIOH_2),
	GPIO_GROUP(GPIOH_3),
	GPIO_GROUP(GPIOH_4),
	GPIO_GROUP(GPIOH_5),
	GPIO_GROUP(GPIOH_6),
	GPIO_GROUP(GPIOH_7),
	GPIO_GROUP(GPIOH_8),
	GPIO_GROUP(GPIOH_9),
	GPIO_GROUP(GPIOH_10),
	GPIO_GROUP(GPIOH_11),
	GPIO_GROUP(GPIOH_12),
	GPIO_GROUP(GPIOH_13),
	GPIO_GROUP(GPIOH_14),
	GPIO_GROUP(GPIOH_15),
	GPIO_GROUP(GPIOH_16),
	GPIO_GROUP(GPIOH_17),
	GPIO_GROUP(GPIOH_18),
	GPIO_GROUP(GPIOH_19),
	GPIO_GROUP(GPIOH_20),
	GPIO_GROUP(GPIOH_21),

	/* bank B */
	GROUP(emmc_d0,		1),
	GROUP(emmc_d1,		1),
	GROUP(emmc_d2,		1),
	GROUP(emmc_d3,		1),
	GROUP(emmc_d4,		1),
	GROUP(emmc_d5,		1),
	GROUP(emmc_d6,		1),
	GROUP(emmc_d7,		1),
	GROUP(emmc_clk,		1),
	GROUP(emmc_rst,		1),
	GROUP(emmc_cmd,		1),
	GROUP(emmc_ds,		1),
	GROUP(loop_gpio_b12,	1),

	GROUP(spif_hold,	3),
	GROUP(spif_mo,		3),
	GROUP(spif_mi,		3),
	GROUP(spif_clk,		3),
	GROUP(spif_wp,		3),
	GROUP(spif_cs,		3),
	GROUP(remote_out,	3),

	/*bank W */
	GROUP(hdmirx_a_hpd,	1),
	GROUP(hdmirx_a_det,	1),
	GROUP(hdmirx_a_sda,	1),
	GROUP(hdmirx_a_sck,	1),
	GROUP(hdmirx_c_hpd,	1),
	GROUP(hdmirx_c_det,	1),
	GROUP(hdmirx_c_sda,	1),
	GROUP(hdmirx_c_sck,	1),
	GROUP(hdmirx_b_hpd,	1),
	GROUP(hdmirx_b_det,	1),
	GROUP(hdmirx_b_sda,	1),
	GROUP(hdmirx_b_sck,	1),
	GROUP(cec,		1),

	GROUP(uart_b_tx_w2,	2),
	GROUP(uart_b_rx_w3,	2),
	GROUP(uart_b_tx_w6,	2),
	GROUP(uart_b_rx_w7,	2),
	GROUP(uart_b_tx_w10,	2),
	GROUP(uart_b_rx_w11,	2),

	/* bank Z */
	GROUP(tdm_fs2,		1),
	GROUP(tdm_sclk2,	1),
	GROUP(tdm_d4,		1),
	GROUP(tdm_d5,		1),
	GROUP(tdm_d6,		1),
	GROUP(tdm_d7,		1),
	GROUP(mclk_2,		1),

	GROUP(tsin_a_clk,	2),
	GROUP(tsin_a_sop,	2),
	GROUP(tsin_a_valid,	2),
	GROUP(tsin_a_d0,	2),

	GROUP(i2c0_sck_z,	3),
	GROUP(i2c0_sda_z,	3),
	GROUP(atv_if_agc,	3),

	GROUP(spi0_miso_z,	4),
	GROUP(spi0_mosi_z,	4),
	GROUP(spi0_clk_z,	4),
	GROUP(spi0_ss0_z,	4),
	GROUP(spi0_ss1_z,	4),
	GROUP(spi0_ss2_z,	4),
	GROUP(dtv_if_agc,	4),

	GROUP(uart_a_tx,	5),
	GROUP(uart_a_rx,	5),
	GROUP(uart_a_cts,	5),
	GROUP(uart_a_rts,	5),
	GROUP(pwm_b_z,		5),
	GROUP(pwm_f_z,		5),
	GROUP(pwm_e_z,		5),

	GROUP(pdm_din1_z1,	6),
	GROUP(pdm_dclk_z,	6),
	GROUP(pdm_din0_z,	6),
	GROUP(pdm_din1_z6,	6),

	/* bank H */
	GROUP(tcon_0,		1),
	GROUP(tcon_1,		1),
	GROUP(tcon_2,		1),
	GROUP(tcon_3,		1),
	GROUP(tcon_4,		1),
	GROUP(tcon_5,		1),
	GROUP(tcon_6,		1),
	GROUP(tcon_7,		1),
	GROUP(tcon_8,		1),
	GROUP(tcon_9,		1),
	GROUP(tcon_10,		1),
	GROUP(tcon_11,		1),
	GROUP(tcon_12,		1),
	GROUP(tcon_13,		1),
	GROUP(tcon_14,		1),
	GROUP(tcon_15,		1),
	GROUP(hsync,		1),
	GROUP(vsync,		1),
	GROUP(i2c1_sck_h,	1),
	GROUP(i2c1_sda_h,	1),

	GROUP(tcon_lock,	2),
	GROUP(uart_c_tx_h,	2),
	GROUP(uart_c_rx_h,	2),
	GROUP(uart_c_cts_h,	2),
	GROUP(uart_c_rts_h,	2),
	GROUP(sync_3d_out,	2),
	GROUP(tdm_d3_h6,	2),
	GROUP(spi0_ss0_h,	2),
	GROUP(spi0_ss1_h,	2),
	GROUP(spi0_miso_h,	2),
	GROUP(spi0_mosi_h,	2),
	GROUP(spi0_clk_h,	2),
	GROUP(spi0_ss2_h,	2),
	GROUP(tdm_d3_h13,	2),
	GROUP(mclk_1,		2),
	GROUP(tdm_sclk1,	2),
	GROUP(tdm_fs1,		2),
	GROUP(tdm_d0,		2),
	GROUP(tdm_d1,		2),
	GROUP(tdm_d2,		2),
	GROUP(loop_gpio_b13,	2),

	GROUP(tcon_sfc_h0,	3),
	GROUP(tcon_sfc_h8,	3),
	GROUP(i2c2_sck_h,	3),
	GROUP(i2c2_sda_h,	3),
	GROUP(pwm_vs_h12,	3),
	GROUP(pwm_vs_h13,	3),
	GROUP(pdm_din1_h14,	3),
	GROUP(pdm_din0_h18,	3),
	GROUP(pdm_dclk_h,	3),

	GROUP(pwm_d,		4),
	GROUP(vx1_htpdn,	4),
	GROUP(vx1_lockn,	4),
	GROUP(pwm_e_h,		4),
	GROUP(pwm_f_h,		4),
	GROUP(pwm_b_h,		4),
	GROUP(eth_act_led,	4),
	GROUP(eth_link_led,	4)
};

static const char * const gpio_periphs_groups[] = {
	"GPIOB_0", "GPIOB_1", "GPIOB_2", "GPIOB_3",
	"GPIOB_4", "GPIOB_5", "GPIOB_6", "GPIOB_7",
	"GPIOB_8", "GPIOB_9", "GPIOB_10", "GPIOB_11",
	"GPIOB_12", "GPIOB_13",

	"GPIOW_0", "GPIOW_1", "GPIOW_2", "GPIOW_3", "GPIOW_4",
	"GPIOW_5", "GPIOW_6", "GPIOW_7", "GPIOW_8", "GPIOW_9",
	"GPIOW_10", "GPIOW_11", "GPIOW_12",

	"GPIOZ_0", "GPIOZ_1", "GPIOZ_2", "GPIOZ_3", "GPIOZ_4",
	"GPIOZ_5", "GPIOZ_6",

	"GPIOH_0", "GPIOH_1", "GPIOH_2", "GPIOH_3", "GPIOH_4",
	"GPIOH_5", "GPIOH_6", "GPIOH_7", "GPIOH_8", "GPIOH_9",
	"GPIOH_10", "GPIOH_11", "GPIOH_12", "GPIOH_13", "GPIOH_14",
	"GPIOH_15", "GPIOH_16", "GPIOH_17", "GPIOH_18", "GPIOH_19",
	"GPIOH_20", "GPIOH_21"
};

static const char * const emmc_groups[] = {
	"emmc_d0", "emmc_d1", "emmc_d2", "emmc_d3",
	"emmc_d4", "emmc_d5", "emmc_d6", "emmc_d7",
	"emmc_clk", "emmc_rst", "emmc_cmd", "emmc_ds"
};

static const char * const spif_groups[] = {
	"spif_hold", "spif_mo", "spif_mi", "spif_clk",
	"spif_wp", "spif_cs"
};

static const char * const remote_out_groups[] = {
	"remote_out"
};

static const char * const hdmirx_a_groups[] = {
	"hdmirx_a_hpd", "hdmirx_a_det", "hdmirx_a_sda", "hdmirx_a_sck"
};

static const char * const hdmirx_b_groups[] = {
	"hdmirx_b_hpd", "hdmirx_b_det", "hdmirx_b_sda", "hdmirx_b_sck"

};

static const char * const hdmirx_c_groups[] = {
	"hdmirx_c_hpd", "hdmirx_c_det", "hdmirx_c_sda", "hdmirx_c_sck"
};

static const char * const cec_groups[] = {
	"cec"
};

static const char * const uart_a_groups[] = {
	"uart_a_tx", "uart_a_rx", "uart_a_cts", "uart_a_rts"
};

static const char * const uart_b_groups[] = {
	"uart_b_tx_w2", "uart_b_rx_w3", "uart_b_tx_w6", "uart_b_rx_w7",
	"uart_b_tx_w10", "uart_b_rx_w11"

};

static const char * const uart_c_groups[] = {
	"uart_c_tx_h", "uart_c_rx_h", "uart_c_cts_h", "uart_c_rts_h"
};

static const char * const tdm_groups[] = {
	"tdm_fs2", "tdm_sclk2", "tdm_d4", "tdm_d5",
	"tdm_d6", "tdm_d7", "tdm_d3_h6", "tdm_d3_h13",
	"tdm_sclk1", "tdm_fs1", "tdm_d0", "tdm_d1",
	"tdm_d2"
};

static const char * const tsin_a_groups[] = {
	"tsin_a_clk", "tsin_a_sop", "tsin_a_valid", "tsin_a_d0"
};

static const char * const i2c0_groups[] = {
	"i2c0_sck_z", "i2c0_sda_z"
};

static const char * const i2c1_groups[] = {
	"i2c1_sck_h", "i2c1_sda_h"
};

static const char * const i2c2_groups[] = {
	"i2c2_sck_h", "i2c2_sda_h"
};

static const char * const atv_groups[] = {
	"atv_if_agc"
};

static const char * const dtv_groups[] = {
	"dtv_if_agc"
};

static const char * const pwm_d_groups[] = {
	"pwm_d"
};

static const char * const pwm_b_groups[] = {
	"pwm_b_z", "pwm_b_h"
};

static const char * const pwm_e_groups[] = {
	"pwm_e_z", "pwm_e_h"
};

static const char * const pwm_f_groups[] = {
	"pwm_f_z", "pwm_f_h"
};

static const char * const pwm_vs_groups[] = {
	"pwm_vs_h12", "pwm_vs_h13"
};

static const char * const pdm_groups[] = {
	"pdm_din1_z1", "pdm_dclk_z", "pdm_din0_z", "pdm_din1_z6",
	"pdm_din1_h14", "pdm_din0_h18", "pdm_dclk_h"
};

static const char * const tcon_groups[] = {
	"tcon_0", "tcon_1", "tcon_2", "tcon_3",
	"tcon_4", "tcon_5", "tcon_6", "tcon_7",
	"tcon_8", "tcon_9", "tcon_10", "tcon_11",
	"tcon_12", "tcon_13", "tcon_14", "tcon_15",
	"tcon_lock", "tcon_sfc_h0", "tcon_sfc_h8"
};

static const char * const hsync_groups[] = {
	"hsync"
};

static const char * const vsync_groups[] = {
	"vsync"
};

static const char * const sync_3d_out_groups[] = {
	"sync_3d_out"
};

static const char * const spi0_groups[] = {
	"spi0_ss0_h", "spi0_ss1_h", "spi0_miso_h", "spi0_mosi_h",
	"spi0_clk_h", "spi0_ss2_h",

	"spi0_miso_z", "spi0_mosi_z", "spi0_clk_z", "spi0_ss0_z",
	"spi0_ss1_z", "spi0_ss2_z"
};

static const char * const mclk_groups[] = {
	"mclk_1"
};

static const char * const vx1_groups[] = {
	"vx1_htpdn", "vx1_lockn"
};

/* remove ? */
static const char * const eth_groups[] = {
	"eth_act_led", "eth_link_led"
};

static struct meson_pmx_func meson_t5_periphs_functions[] = {
	FUNCTION(gpio_periphs),
	FUNCTION(emmc),
	FUNCTION(spif),
	FUNCTION(remote_out),
	FUNCTION(hdmirx_a),
	FUNCTION(hdmirx_b),
	FUNCTION(hdmirx_c),
	FUNCTION(cec),
	FUNCTION(uart_a),
	FUNCTION(uart_b),
	FUNCTION(uart_c),
	FUNCTION(tdm),
	FUNCTION(tsin_a),
	FUNCTION(i2c0),
	FUNCTION(i2c1),
	FUNCTION(i2c2),
	FUNCTION(atv),
	FUNCTION(dtv),
	FUNCTION(spi0),
	FUNCTION(pwm_b),
	FUNCTION(pwm_d),
	FUNCTION(pwm_e),
	FUNCTION(pwm_f),
	FUNCTION(pwm_vs),
	FUNCTION(pdm),
	FUNCTION(tcon),
	FUNCTION(hsync),
	FUNCTION(vsync),
	FUNCTION(sync_3d_out),
	FUNCTION(mclk),
	FUNCTION(vx1),
	FUNCTION(eth)
};

static struct meson_bank meson_t5_periphs_banks[] = {
	/* name  first  last  irq  pullen  pull  dir  out  in */
	BANK("B",    GPIOB_0,    GPIOB_13, 33,
	     0,  0,  0,  0,  0,  0,  1, 0,  2, 0),
	BANK("W",    GPIOW_0,    GPIOW_12, 54,
	     3,  0,  3,  0,  9,  0,  10,  0,  11,  0),
	BANK("Z",    GPIOZ_0,    GPIOZ_6,  47,
	     1,  0,  1,  0,  3,  0,  4, 0,  5, 0),
	BANK("H",    GPIOH_0,    GPIOH_21,  11,
	     2,  0,  2,  0,  6,  0,  7, 0,  8, 0)
};

static struct meson_bank meson_t5_aobus_banks[] = {
	/* name  first  last  irq  pullen  pull  dir  out  in  */
	BANK("D",   GPIOD_0,  GPIOD_10,  0,
	     3,  0,  2, 0,  0,  0,  4, 0,  1,  0),
	BANK("E",   GPIOE_0,  GPIOE_2,   67,
	     3,  16,  2, 16,  0,  16,  4, 16,  1,  16)
};

static struct meson_bank meson_t5_analog_banks[] = {
	/* name  first  last  irq  pullen  pull  dir  out  in  */
	BANK("ANALOG",  CDAC_IOUT,  CVBS1,  70,
	     0,  31,  0, 30,  0,  0,  0, 0,  1,  0)
};

static struct meson_pmx_bank meson_t5_analog_pmx_banks[] = {
	/*	 name	 first		lask	   reg	offset  */
	BANK_PMX("ANALOG", CDAC_IOUT, CVBS1, 0x0, 29)
};

static struct meson_axg_pmx_data meson_t5_analog_pmx_banks_data = {
	.pmx_banks	= meson_t5_analog_pmx_banks,
	.num_pmx_banks	= ARRAY_SIZE(meson_t5_analog_pmx_banks),
};

static struct meson_bank meson_t5_analog_banks[] = {
	/* name  first  last  irq  pullen  pull  dir  out  in  */
	BANK("ANALOG",  CDAC_IOUT,  CVBS1,  70,
	     0,  31,  0, 30,  0,  0,  0, 0,  1,  0)
};

static struct meson_pmx_bank meson_t5_analog_pmx_banks[] = {
	/*	 name	 first		lask	   reg	offset  */
	BANK_PMX("ANALOG", CDAC_IOUT, CVBS1, 0x0, 29)
};

static struct meson_axg_pmx_data meson_t5_analog_pmx_banks_data = {
	.pmx_banks	= meson_t5_analog_pmx_banks,
	.num_pmx_banks	= ARRAY_SIZE(meson_t5_analog_pmx_banks),
};

static struct meson_pmx_bank meson_t5_periphs_pmx_banks[] = {
	/*	 name	 first		lask	   reg	offset  */
	BANK_PMX("B",    GPIOB_0, GPIOB_13, 0x0, 0),
	BANK_PMX("W",    GPIOW_0, GPIOW_12, 0x2, 0),
	BANK_PMX("Z",    GPIOZ_0, GPIOZ_6,  0x4, 0),
	BANK_PMX("H",    GPIOH_0, GPIOH_21, 0x5, 0)
};

static struct meson_axg_pmx_data meson_t5_periphs_pmx_banks_data = {
	.pmx_banks	= meson_t5_periphs_pmx_banks,
	.num_pmx_banks	= ARRAY_SIZE(meson_t5_periphs_pmx_banks),
};

static struct meson_drive_bank meson_t5_periphs_drive_banks[] = {
	/*	 name	 first		lask	   reg	offset  */
	BANK_DRIVE("B",    GPIOB_0, GPIOB_13, 0x0, 0),
	BANK_DRIVE("W",    GPIOW_0, GPIOW_12, 0x3, 0),
	BANK_DRIVE("Z",	   GPIOZ_0, GPIOZ_6,  0x1, 0),
	BANK_DRIVE("H",    GPIOH_0, GPIOH_21, 0x2, 0)
};

static struct meson_drive_data meson_t5_periphs_drive_data = {
	.drive_banks	= meson_t5_periphs_drive_banks,
	.num_drive_banks = ARRAY_SIZE(meson_t5_periphs_drive_banks),
};

static struct meson_pmx_bank meson_t5_aobus_pmx_banks[] = {
	BANK_PMX("D", GPIOD_0, GPIOD_10, 0x0, 0),
	BANK_PMX("E", GPIOE_0, GPIOE_2, 0x1, 16)
};

static struct meson_axg_pmx_data meson_t5_aobus_pmx_banks_data = {
	.pmx_banks	= meson_t5_aobus_pmx_banks,
	.num_pmx_banks	= ARRAY_SIZE(meson_t5_aobus_pmx_banks),
};

static struct meson_drive_bank meson_t5_aobus_drive_banks[] = {
	BANK_DRIVE("D", GPIOD_0, GPIOD_10, 0x0, 0),
	BANK_DRIVE("E", GPIOE_0, GPIOE_2, 0x1, 0)
};

static struct meson_drive_data meson_t5_aobus_drive_data = {
	.drive_banks     = meson_t5_aobus_drive_banks,
	.num_drive_banks = ARRAY_SIZE(meson_t5_aobus_drive_banks),
};

static struct meson_pinctrl_data meson_t5_periphs_pinctrl_data = {
	.name		= "periphs-banks",
	.init		= NULL,
	.pins		= meson_t5_periphs_pins,
	.groups		= meson_t5_periphs_groups,
	.funcs		= meson_t5_periphs_functions,
	.banks		= meson_t5_periphs_banks,
	.num_pins	= ARRAY_SIZE(meson_t5_periphs_pins),
	.num_groups	= ARRAY_SIZE(meson_t5_periphs_groups),
	.num_funcs	= ARRAY_SIZE(meson_t5_periphs_functions),
	.num_banks	= ARRAY_SIZE(meson_t5_periphs_banks),
	.pmx_ops	= &meson_axg_pmx_ops,
	.pmx_data	= &meson_t5_periphs_pmx_banks_data,
	.pcf_ops	= &meson_g12a_pinconf_ops,
	.pcf_drv_data	= &meson_t5_periphs_drive_data,
};

static struct meson_pinctrl_data meson_t5_aobus_pinctrl_data = {
	.name		= "aobus-banks",
	.init		= NULL,
	.pins		= meson_t5_aobus_pins,
	.groups		= meson_t5_aobus_groups,
	.funcs		= meson_t5_aobus_functions,
	.banks		= meson_t5_aobus_banks,
	.num_pins	= ARRAY_SIZE(meson_t5_aobus_pins),
	.num_groups	= ARRAY_SIZE(meson_t5_aobus_groups),
	.num_funcs	= ARRAY_SIZE(meson_t5_aobus_functions),
	.num_banks	= ARRAY_SIZE(meson_t5_aobus_banks),
	.pmx_ops	= &meson_axg_pmx_ops,
	.pmx_data	= &meson_t5_aobus_pmx_banks_data,
	.pcf_ops	= &meson_g12a_pinconf_ops,
	.pcf_drv_data	= &meson_t5_aobus_drive_data,
};

static struct meson_pinctrl_data meson_t5_analog_pinctrl_data = {
	.name		= "analog-banks",
	.init		= NULL,
	.pins		= meson_t5_analog_pins,
	.groups		= meson_t5_analog_groups,
	.funcs		= meson_t5_analog_functions,
	.banks		= meson_t5_analog_banks,
	.num_pins	= ARRAY_SIZE(meson_t5_analog_pins),
	.num_groups	= ARRAY_SIZE(meson_t5_analog_groups),
	.num_funcs	= ARRAY_SIZE(meson_t5_analog_functions),
	.num_banks	= ARRAY_SIZE(meson_t5_analog_banks),
	.pmx_ops	= &meson_axg_pmx_ops,
	.pmx_data	= &meson_t5_analog_pmx_banks_data,
};

static const struct of_device_id meson_t5_pinctrl_dt_match[] = {
	{
		.compatible = "amlogic,meson-t5-periphs-pinctrl",
		.data = &meson_t5_periphs_pinctrl_data,
	},
	{
		.compatible = "amlogic,meson-t5-aobus-pinctrl",
		.data = &meson_t5_aobus_pinctrl_data,
	},
	{
		.compatible = "amlogic,meson-t5-analog-pinctrl",
		.data = &meson_t5_analog_pinctrl_data,
	},
	{ }
};

static struct platform_driver meson_t5_pinctrl_driver = {
	.probe  = meson_pinctrl_probe,
	.driver = {
		.name	= "meson-t5-pinctrl",
		.of_match_table = meson_t5_pinctrl_dt_match,
	},
};

static int __init t5_pmx_init(void)
{
	return platform_driver_register(&meson_t5_pinctrl_driver);
}

static void __exit t5_pmx_exit(void)
{
	platform_driver_unregister(&meson_t5_pinctrl_driver);
}

arch_initcall(t5_pmx_init);
module_exit(t5_pmx_exit);
MODULE_DESCRIPTION("t5 pin control driver");
MODULE_LICENSE("GPL v2");


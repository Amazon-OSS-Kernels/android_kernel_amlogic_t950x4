/*
 * drivers/amlogic/media/vout/backlight/aml_ldim/ldim_drv.c
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

#include <linux/version.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/major.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/amlogic/iomap.h>
#include <linux/workqueue.h>
#include <linux/amlogic/media/vfm/vframe.h>
#include <linux/amlogic/media/vout/lcd/aml_ldim.h>
#include <linux/amlogic/media/vout/lcd/aml_bl.h>
#include <linux/amlogic/media/vout/vout_notify.h>
#include <linux/amlogic/media/vout/lcd/lcd_vout.h>
#include <linux/amlogic/media/utils/vdec_reg.h>
#include <linux/amlogic/media/vout/lcd/lcd_unifykey.h>
#include <linux/amlogic/media/vout/lcd/ldim_alg.h>
#include <linux/of_reserved_mem.h>
#include "ldim_drv.h"
#include "ldim_reg.h"

#define AML_LDIM_DEV_NAME            "aml_ldim"

const char ldim_dev_id[] = "ldim-dev";
unsigned char ldim_debug_print;

struct ldim_dev_s {
	struct ldim_operate_func_s *ldim_op_func;

	struct cdev   cdev;
	struct device *dev;
	dev_t aml_ldim_devno;
	struct class *aml_ldim_clsp;
};

static struct ldim_dev_s ldim_dev;
static struct aml_ldim_driver_s ldim_driver;

static unsigned char ldim_level_update;
static unsigned int brightness_vs_cnt;

static spinlock_t  ldim_isr_lock;
static spinlock_t  rdma_ldim_isr_lock;

static struct workqueue_struct *ldim_queue;
static struct work_struct ldim_on_vs_work;
static struct work_struct ldim_off_vs_work;

static int ldim_on_init(void);
static int ldim_power_on(void);
static int ldim_power_off(void);
static int ldim_set_level(unsigned int level);
static void ldim_test_ctrl(int flag);

static struct aml_ldim_info_s ldim_info;

static struct ldim_config_s ldim_config = {
	.hsize = 3840,
	.vsize = 2160,
	.hist_row = 1,
	.hist_col = 1,
	.bl_mode = 1,
	.bl_en = 1,
	.hvcnt_bypass = 0,
	.dev_index = 0xff,
	.ldim_info = &ldim_info,
};

static struct ldim_rmem_s ldim_rmem = {
	.wr_mem_vaddr1 = NULL,
	.wr_mem_paddr1 = 0,
	.wr_mem_vaddr2 = NULL,
	.wr_mem_paddr2 = 0,
	.rd_mem_vaddr1 = NULL,
	.rd_mem_paddr1 = 0,
	.wr_mem_vaddr2 = NULL,
	.wr_mem_paddr2 = 0,
	.wr_mem_size = 0x3000,
	.rd_mem_size = 0x1000,
};

static struct aml_ldim_driver_s ldim_driver = {
	.valid_flag = 0, /* default invalid, active when bl_ctrl_method=ldim */
	.static_pic_flag = 0,
	.vsync_change_flag = 0,

	.init_on_flag = 0,
	.func_en = 0,
	.remap_en = 0,
	.demo_en = 0,
	.func_bypass = 0,
	.brightness_bypass = 0,
	.test_en = 0,
	.avg_update_en = 0,
	.matrix_update_en = 0,
	.alg_en = 0,
	.top_en = 0,
	.hist_en = 0,
	.load_db_en = 1,
	.db_print_flag = 0,

	.data_min = LD_DATA_MIN,
	.data_max = LD_DATA_MAX,
	.brightness_level = 0,
	.litgain = LD_DATA_MAX,
	.irq_cnt = 0,

	.conf = &ldim_config,
	.ldev_conf = NULL,
	.rmem = &ldim_rmem,
	.hist_matrix = NULL,
	.max_rgb = NULL,
	.test_matrix = NULL,
	.local_ldim_matrix = NULL,
	.ldim_matrix_buf = NULL,
	.fw_para = NULL,

	.init = ldim_on_init,
	.power_on = ldim_power_on,
	.power_off = ldim_power_off,
	.set_level = ldim_set_level,
	.test_ctrl = ldim_test_ctrl,

	.config_print = NULL,
	.pinmux_ctrl = NULL,
	.pwm_vs_update = NULL,
	.device_power_on = NULL,
	.device_power_off = NULL,
	.device_bri_update = NULL,
	.device_bri_check = NULL,
};

struct aml_ldim_driver_s *aml_ldim_get_driver(void)
{
	return &ldim_driver;
}

void ldim_delay(int ms)
{
	if (ms > 0 && ms < 20)
		usleep_range(ms * 1000, ms * 1000 + 1);
	else if (ms > 20)
		msleep(ms);
}

void ldim_remap_ctrl(unsigned char status)
{
	struct aml_bl_drv_s *bl_drv = aml_bl_get_driver();
	unsigned int temp;

	temp = ldim_driver.matrix_update_en;
	if (status) {
		ldim_driver.matrix_update_en = 0;
		ldim_hw_remap_en(1);
		msleep(20);
		if (bl_drv->data->chip_type == BL_CHIP_TM2)
			ldim_hw_vpu_dma_mif_en(LDIM_VPU_DMA_RD, 1);
		ldim_driver.matrix_update_en = temp;
	} else {
		ldim_driver.matrix_update_en = 0;
		ldim_hw_remap_en(0);
		msleep(20);
		if (bl_drv->data->chip_type == BL_CHIP_TM2)
			ldim_hw_vpu_dma_mif_en(LDIM_VPU_DMA_RD, 0);
		ldim_driver.matrix_update_en = temp;
	}
	LDIMPR("%s: %d\n", __func__, status);
}

void ldim_func_ctrl(unsigned char status)
{
	if (status) {
		/* enable other flag */
		ldim_driver.top_en = 1;
		ldim_driver.hist_en = 1;
		ldim_driver.alg_en = 1;
		/* enable update */
		ldim_driver.avg_update_en = 1;
		/*ldim_driver.matrix_update_en = 1;*/

		ldim_remap_ctrl(ldim_driver.remap_en);
	} else {
		/* disable remap */
		ldim_remap_ctrl(0);
		/* disable update */
		ldim_driver.avg_update_en = 0;
		/*ldim_driver.matrix_update_en = 0;*/
		/* disable other flag */
		ldim_driver.top_en = 0;
		ldim_driver.hist_en = 0;
		ldim_driver.alg_en = 0;

		/* refresh system brightness */
		ldim_level_update = 1;
	}
	LDIMPR("%s: %d\n", __func__, status);
}

void ldim_stts_initial(unsigned int pic_h, unsigned int pic_v,
		       unsigned int hist_vnum, unsigned int hist_hnum)
{
	LDIMPR("%s: %d %d %d %d\n",
	       __func__, pic_h, pic_v, hist_vnum, hist_hnum);

	ldim_driver.fw_para->hist_col = hist_hnum;
	ldim_driver.fw_para->hist_row = hist_vnum;

	if (!ldim_dev.ldim_op_func) {
		LDIMERR("%s: invalid ldim_op_func\n", __func__);
		return;
	}
	if (ldim_dev.ldim_op_func->stts_init) {
		ldim_dev.ldim_op_func->stts_init(pic_h, pic_v,
						 hist_vnum, hist_hnum);
	}
}

void ldim_initial(unsigned int pic_h, unsigned int pic_v,
		  unsigned int hist_vnum, unsigned int hist_hnum,
		  unsigned int blk_mode, unsigned int ldim_bl_en,
		  unsigned int hvcnt_bypass)
{
	if (!ldim_driver.fw_para->nprm) {
		LDIMERR("%s: nprm is null\n", __func__);
		return;
	}
	LDIMPR("%s: %d %d %d %d %d %d %d\n",
	       __func__, pic_h, pic_v, hist_vnum, hist_hnum,
	       blk_mode, ldim_bl_en, hvcnt_bypass);

	ldim_driver.matrix_update_en = ldim_bl_en;
	ld_func_cfg_ldreg(ldim_driver.fw_para->nprm);
	/* config params begin */
	/* configuration of the panel parameters */
	ldim_driver.fw_para->nprm->reg_LD_pic_RowMax = pic_v;
	ldim_driver.fw_para->nprm->reg_LD_pic_ColMax = pic_h;
	/* Maximum to BLKVMAX  , Maximum to BLKHMAX */
	ldim_driver.fw_para->nprm->reg_LD_BLK_Vnum     = hist_vnum;
	ldim_driver.fw_para->nprm->reg_LD_BLK_Hnum     = hist_hnum;
	ldim_driver.fw_para->nprm->reg_LD_BackLit_mode = blk_mode;
	/*config params end */
	ld_func_fw_cfg_once(ldim_driver.fw_para->nprm);

	if (!ldim_dev.ldim_op_func) {
		LDIMERR("%s: invalid ldim_op_func\n", __func__);
		return;
	}
	if (ldim_dev.ldim_op_func->remap_init) {
		ldim_dev.ldim_op_func->remap_init(ldim_driver.fw_para->nprm,
						  ldim_bl_en,
						  hvcnt_bypass);
	}
}

static int ldim_on_init(void)
{
	int ret = 0;

	if (ldim_debug_print)
		LDIMPR("%s\n", __func__);

	/* init ldim */
	ldim_stts_initial(ldim_config.hsize, ldim_config.vsize,
			  ldim_config.hist_row, ldim_config.hist_col);
	ldim_initial(ldim_config.hsize, ldim_config.vsize,
		     ldim_config.hist_row, ldim_config.hist_col,
		     ldim_config.bl_mode, 1, 0);

	ldim_func_ctrl(0); /* default disable ldim function */

	if (ldim_driver.pinmux_ctrl)
		ldim_driver.pinmux_ctrl(1);
	ldim_driver.init_on_flag = 1;
	ldim_level_update = 1;

	return ret;
}

static int ldim_power_on(void)
{
	int ret = 0;

	LDIMPR("%s\n", __func__);

	/* init ldim */
	ldim_stts_initial(ldim_config.hsize, ldim_config.vsize,
			  ldim_config.hist_row, ldim_config.hist_col);
	ldim_initial(ldim_config.hsize, ldim_config.vsize,
		     ldim_config.hist_row, ldim_config.hist_col,
		     ldim_config.bl_mode, 1, 0);
	ldim_func_ctrl(ldim_driver.func_en);

	if (ldim_driver.device_power_on)
		ldim_driver.device_power_on();
	ldim_driver.init_on_flag = 1;
	ldim_level_update = 1;

	return ret;
}

static int ldim_power_off(void)
{
	int ret = 0;

	LDIMPR("%s\n", __func__);

	ldim_driver.init_on_flag = 0;
	if (ldim_driver.device_power_off)
		ldim_driver.device_power_off();

	ldim_func_ctrl(0);

	return ret;
}

static int ldim_set_level(unsigned int level)
{
	struct aml_bl_drv_s *bl_drv = aml_bl_get_driver();
	unsigned int level_max, level_min;

	ldim_driver.brightness_level = level;

	if (ldim_driver.brightness_bypass)
		return 0;

	level_max = bl_drv->bconf->level_max;
	level_min = bl_drv->bconf->level_min;

	level = ((level - level_min) *
		 (ldim_driver.data_max - ldim_driver.data_min)) /
		(level_max - level_min) + ldim_driver.data_min;
	level &= 0xfff;
	ldim_driver.litgain = (unsigned long)level;
	ldim_level_update = 1;

	return 0;
}

static void ldim_test_ctrl(int flag)
{
	if (flag) /* when enable lcd bist pattern, bypass ldim function */
		ldim_driver.func_bypass = 1;
	else
		ldim_driver.func_bypass = 0;
	LDIMPR("%s: ldim_func_bypass = %d\n",
	       __func__, ldim_driver.func_bypass);
}

/* ******************************************************
 * local dimming function
 * ******************************************************
 */
static void ldim_remap_update(void)
{
	if (!ldim_dev.ldim_op_func) {
		if (brightness_vs_cnt == 0)
			LDIMERR("%s: invalid ldim_op_func\n", __func__);
		return;
	}
	if (!ldim_dev.ldim_op_func->remap_update)
		return;

	ldim_dev.ldim_op_func->remap_update(ldim_driver.fw_para->nprm,
					    ldim_driver.avg_update_en,
					    ldim_driver.matrix_update_en);
}

static irqreturn_t ldim_vsync_isr(int irq, void *dev_id)
{
	unsigned long flags;

	if (ldim_driver.init_on_flag == 0)
		return IRQ_HANDLED;

	spin_lock_irqsave(&ldim_isr_lock, flags);

	if (brightness_vs_cnt++ >= 30) /* for debug print */
		brightness_vs_cnt = 0;

	if (ldim_driver.func_en) {
		if (ldim_driver.hist_en) {
			/*schedule_work(&ldim_on_vs_work);*/
			queue_work(ldim_queue, &ldim_on_vs_work);
		}
		if (ldim_driver.avg_update_en)
			ldim_remap_update();
	} else {
		/*schedule_work(&ldim_off_vs_work);*/
		queue_work(ldim_queue, &ldim_off_vs_work);
	}

	ldim_driver.irq_cnt++;
	if (ldim_driver.irq_cnt > 0xfffffff)
		ldim_driver.irq_cnt = 0;

	spin_unlock_irqrestore(&ldim_isr_lock, flags);

	return IRQ_HANDLED;
}

static void ldim_on_vs_brightness(void)
{
	struct LDReg_s *nprm = ldim_driver.fw_para->nprm;
	unsigned int size;
	unsigned int i;

	if (!nprm)
		return;
	if (ldim_driver.init_on_flag == 0)
		return;

	if (ldim_driver.func_bypass)
		return;

	if (!ldim_driver.fw_para->fw_alg_frm) {
		if (brightness_vs_cnt == 0)
			LDIMERR("%s: ldim_alg ko is not installed\n", __func__);
		return;
	}

	size = ldim_driver.conf->hist_row * ldim_driver.conf->hist_col;

	if (ldim_driver.test_en) {
		for (i = 0; i < size; i++) {
			ldim_driver.local_ldim_matrix[i] =
				(unsigned short)nprm->BL_matrix[i];
			ldim_driver.ldim_matrix_buf[i] =
				ldim_driver.test_matrix[i];
		}
	} else {
		for (i = 0; i < size; i++) {
			ldim_driver.local_ldim_matrix[i] =
				(unsigned short)nprm->BL_matrix[i];
			ldim_driver.ldim_matrix_buf[i] =
				(unsigned short)
				(((nprm->BL_matrix[i] * ldim_driver.litgain)
				+ (LD_DATA_MAX / 2)) >> LD_DATA_DEPTH);
		}
	}

	if (ldim_driver.device_bri_update) {
		ldim_driver.device_bri_update(ldim_driver.ldim_matrix_buf,
					      size);
	} else {
		if (brightness_vs_cnt == 0)
			LDIMERR("%s: device_bri_update is null\n", __func__);
	}
}

static void ldim_off_vs_brightness(void)
{
	struct LDReg_s *nprm = ldim_driver.fw_para->nprm;
	unsigned int size;
	unsigned int i;
	int ret;

	if (!nprm)
		return;
	if (ldim_driver.init_on_flag == 0)
		return;

	size = ldim_driver.conf->hist_row * ldim_driver.conf->hist_col;

	if (ldim_level_update) {
		ldim_level_update = 0;
		if (ldim_debug_print) {
			LDIMPR("%s: level update: 0x%x\n",
			       __func__, ldim_driver.litgain);
		}
		for (i = 0; i < size; i++) {
			ldim_driver.local_ldim_matrix[i] =
				(unsigned short)nprm->BL_matrix[i];
			ldim_driver.ldim_matrix_buf[i] =
				(unsigned short)(ldim_driver.litgain);
		}
	} else {
		if (ldim_driver.device_bri_check) {
			ret = ldim_driver.device_bri_check();
			if (ret) {
				if (brightness_vs_cnt == 0) {
					LDIMERR("%s: device_bri_check error\n",
						__func__);
				}
				ldim_level_update = 1;
			}
		}
		return;
	}

	if (ldim_driver.device_bri_update) {
		ldim_driver.device_bri_update(ldim_driver.ldim_matrix_buf,
					      size);
	} else {
		if (brightness_vs_cnt == 0)
			LDIMERR("%s: device_bri_update is null\n", __func__);
	}
}

void ldim_on_vs_arithmetic_tm2(void)
{
	unsigned int *local_ldim_buf;
	int zone_num;
	int i;

	if (!ldim_driver.hist_matrix)
		return;
	if (!ldim_driver.max_rgb)
		return;

	zone_num = ldim_driver.conf->hist_row * ldim_driver.conf->hist_col;
	local_ldim_buf = kcalloc(zone_num * 20,
				 sizeof(unsigned int), GFP_KERNEL);
	if (!local_ldim_buf)
		return;

	/*stts_read*/
	memcpy(local_ldim_buf, ldim_rmem.wr_mem_vaddr1,
	       zone_num * 20 * sizeof(unsigned int));
	for (i = 0; i < zone_num; i++) {
		memcpy(&ldim_driver.hist_matrix[i * 16],
		       &local_ldim_buf[i * 20],
		       16 * sizeof(unsigned int));
		memcpy(&ldim_driver.max_rgb[i * 3],
		       &local_ldim_buf[(i * 20) + 16],
		       3 * sizeof(unsigned int));
	}

	if (ldim_driver.alg_en == 0) {
		kfree(local_ldim_buf);
		return;
	}
	if (ldim_driver.fw_para->fw_alg_frm) {
		ldim_driver.fw_para->fw_alg_frm(ldim_driver.fw_para,
						ldim_driver.max_rgb,
						ldim_driver.hist_matrix);
	}

	kfree(local_ldim_buf);
}

static void ldim_on_vs_arithmetic(void)
{
	if (ldim_driver.top_en == 0)
		return;

	ldim_hw_stts_read_zone(ldim_driver.conf->hist_row,
			       ldim_driver.conf->hist_col);

	if (ldim_driver.alg_en == 0)
		return;
	if (ldim_driver.fw_para->fw_alg_frm) {
		ldim_driver.fw_para->fw_alg_frm(ldim_driver.fw_para,
						ldim_driver.max_rgb,
						ldim_driver.hist_matrix);
	}
}

static void ldim_on_update_brightness(struct work_struct *work)
{
	ldim_dev.ldim_op_func->vs_arithmetic();
	ldim_on_vs_brightness();
}

static void ldim_off_update_brightness(struct work_struct *work)
{
	ldim_off_vs_brightness();
}

/* ******************************************************
 * local dimming dummy function for virtual ldim dev
 * ******************************************************
 */
static int ldim_virtual_smr(unsigned short *buf, unsigned char len)
{
	return 0;
}

static int ldim_virtual_power_on(void)
{
	return 0;
}

static int ldim_virtual_power_off(void)
{
	return 0;
}

static int ldim_virtual_driver_update(struct aml_ldim_driver_s *ldim_drv)
{
	ldim_drv->device_power_on = ldim_virtual_power_on;
	ldim_drv->device_power_off = ldim_virtual_power_off;
	ldim_drv->device_bri_update = ldim_virtual_smr;

	return 0;
}

static int ldim_dev_add_virtual_driver(struct aml_ldim_driver_s *ldim_drv)
{
	LDIMPR("%s\n", __func__);

	ldim_virtual_driver_update(ldim_drv);
	ldim_drv->init();

	return 0;
}

static int ldim_open(struct inode *inode, struct file *file)
{
	struct ldim_dev_s *devp;

	LDIMPR("%s\n", __func__);
	/* Get the per-device structure that contains this cdev */
	devp = container_of(inode->i_cdev, struct ldim_dev_s, cdev);
	file->private_data = devp;
	return 0;
}

static int ldim_release(struct inode *inode, struct file *file)
{
	LDIMPR("%s\n", __func__);
	file->private_data = NULL;
	return 0;
}

static void aml_ldim_info_update(void)
{
	int i = 0;
	int j = 0;
	struct ldim_fw_para_s *fw_para = ldim_driver.fw_para;
	struct fw_ctrl_config_s *fw_ctrl;

	ldim_driver.remap_en = ldim_config.ldim_info->remapping_en;

	if (fw_para) {
		if (fw_para->ctrl) {
			fw_ctrl = fw_para->ctrl;
			fw_ctrl->TF_alpha = ldim_config.ldim_info->alpha;
			fw_ctrl->LPF_method =
				ldim_config.ldim_info->LPF_method;
			fw_ctrl->lpf_gain = ldim_config.ldim_info->lpf_gain;
			fw_ctrl->lpf_res = ldim_config.ldim_info->lpf_res;
			fw_ctrl->side_blk_diff_th =
				ldim_config.ldim_info->side_blk_diff_th;
			fw_ctrl->bbd_th = ldim_config.ldim_info->bbd_th;
			fw_ctrl->boost_gain = ldim_config.ldim_info->boost_gain;
			fw_ctrl->rgb_base = ldim_config.ldim_info->rgb_base;
			fw_ctrl->Ld_remap_bypass =
				ldim_config.ldim_info->Ld_remap_bypass;
			fw_ctrl->LD_TF_STEP_TH =
				ldim_config.ldim_info->LD_TF_STEP_TH;
			fw_ctrl->TF_BLK_FRESH_BL =
				ldim_config.ldim_info->TF_BLK_FRESH_BL;
			fw_ctrl->fw_LD_ThTF_l =
				ldim_config.ldim_info->fw_LD_ThTF_l;
			fw_ctrl->fw_rgb_diff_th =
				ldim_config.ldim_info->fw_rgb_diff_th;
		}
		for (i = 0; i < 16; i++)
			fw_para->bl_remap_curve[i] =
				ldim_config.ldim_info->bl_remap_curve[i];

		for (i = 0; i < 16; i++) {
			fw_para->fw_ld_whist[i] =
				ldim_config.ldim_info->fw_ld_whist[i];
		}
	}

	for (i = 0; i < 16; i++) {
		for (j = 0; j < 32; j++)
			ld_remap_lut[i][j] =
			ldim_config.ldim_info->Reg_LD_remap_LUT[i][j];
	}

	ldim_driver.func_en = ldim_config.ldim_info->func_en;
	ldim_func_ctrl(ldim_driver.func_en);
}

static void ldim_remap_lut_print(char *buf, int len)
{
	int i, j;

	LDIMPR("ld_remap_lut:\n");
	for (i = 0; i < 16; i++) {
		LDIMPR("  %d:\n", i);
		len = 0;
		for (j = 0; j < 32; j++) {
			if (j == 16)
				len += sprintf(buf + len, "\n");

			len += sprintf(buf + len, " %d",
				       ldim_info.Reg_LD_remap_LUT[i][j]);
		}
		LDIMPR("%s\n", buf);
	}
}

static long ldim_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int i = 0, len = 0;
	int ret = 0;
	char *curve_buf;
	char *lut_buf;
	void __user *argp;
	int mcd_nr;

	memset(&ldim_info, 0, sizeof(struct aml_ldim_info_s));

	mcd_nr = _IOC_NR(cmd);
	LDIMPR("%s: cmd_dir = 0x%x, cmd_nr = 0x%x\n",
	       __func__, _IOC_DIR(cmd), mcd_nr);

	argp = (void __user *)arg;
	switch (mcd_nr) {
	case AML_LDIM_IOC_NR_GET_INFO:
		if (copy_to_user(argp, &ldim_info,
				 sizeof(struct aml_ldim_info_s))) {
			ret = -EFAULT;
		}
		break;
	case AML_LDIM_IOC_NR_SET_INFO:
		if (copy_from_user(&ldim_info, argp,
				   sizeof(struct aml_ldim_info_s))) {
			ret = -EFAULT;
		} else {
			aml_ldim_info_update();
			if (ldim_debug_print) {
				LDIMPR("set ldim info:\n"
				       "func_en:		%d\n"
				       "remapping_en:		%d\n"
				       "alpha:			%d\n"
				       "LPF_method:		%d\n"
				       "lpf_gain:		%d\n"
				       "lpf_res:		%d\n"
				       "side_blk_diff_th:	%d\n"
				       "bbd_th:			%d\n"
				       "boost_gain:		%d\n"
				       "rgb_base:		%d\n"
				       "Ld_remap_bypass:	%d\n"
				       "LD_TF_STEP_TH:		%d\n"
				       "TF_BLK_FRESH_BL:	%d\n"
				       "TF_FRESH_BL:		%d\n"
				       "fw_LD_ThTF_l:		%d\n"
				       "fw_rgb_diff_th:		%d\n"
				       "fw_ld_thist:		%d\n",
				       ldim_info.func_en,
				       ldim_info.remapping_en,
				       ldim_info.alpha,
				       ldim_info.LPF_method,
				       ldim_info.lpf_gain,
				       ldim_info.lpf_res,
				       ldim_info.side_blk_diff_th,
				       ldim_info.bbd_th,
				       ldim_info.boost_gain,
				       ldim_info.rgb_base,
				       ldim_info.Ld_remap_bypass,
				       ldim_info.LD_TF_STEP_TH,
				       ldim_info.TF_BLK_FRESH_BL,
				       ldim_info.TF_FRESH_BL,
				       ldim_info.fw_LD_ThTF_l,
				       ldim_info.fw_rgb_diff_th,
				       ldim_info.fw_ld_thist);

				/* ldim_bl_remap_curve_print */
				len = 16 * 8 + 20;
				curve_buf = kcalloc(len, sizeof(char),
						    GFP_KERNEL);
				if (!curve_buf)
					return 0;
				LDIMPR("bl_remap_curve:\n");
				len = 0;
				for (i = 0; i < 16; i++)
					len +=
					sprintf(curve_buf + len, "\t%d",
						ldim_info.bl_remap_curve[i]);
				LDIMPR("%s\n", curve_buf);

				LDIMPR("fw_ld_whist:\n");
				len = 0;
				for (i = 0; i < 16; i++)
					len +=
					sprintf(curve_buf + len, "\t%d",
						ldim_info.fw_ld_whist[i]);
				LDIMPR("%s\n", curve_buf);

				/*ldim_bl_remap_lut_print*/
				len = 32 * 8 + 20;
				lut_buf = kcalloc(len, sizeof(char),
						    GFP_KERNEL);
				if (!lut_buf) {
					kfree(curve_buf);
					return 0;
				}
				ldim_remap_lut_print(lut_buf, len);
				kfree(curve_buf);
				kfree(lut_buf);
			}
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long ldim_compat_ioctl(struct file *file, unsigned int cmd,
			      unsigned long arg)
{
	unsigned long ret;

	arg = (unsigned long)compat_ptr(arg);
	ret = ldim_ioctl(file, cmd, arg);
	return ret;
}
#endif

static const struct file_operations ldim_fops = {
	.owner          = THIS_MODULE,
	.open           = ldim_open,
	.release        = ldim_release,
	.unlocked_ioctl = ldim_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = ldim_compat_ioctl,
#endif
};

int aml_ldim_get_config_dts(struct device_node *child)
{
	struct vinfo_s *vinfo = get_current_vinfo();
	unsigned int para[5];
	int ret;

	if (!child) {
		LDIMERR("child device_node is null\n");
		return -1;
	}

	/* default setting */
	ldim_config.hsize = vinfo->width;
	ldim_config.vsize = vinfo->height;

	/* get row & col from dts */
	ret = of_property_read_u32_array(child, "bl_ldim_zone_row_col",
					 para, 2);
	if (ret) {
		ret = of_property_read_u32_array(child,
						 "bl_ldim_region_row_col",
						 para, 2);
		if (ret) {
			LDIMERR("failed to get bl_ldim_zone_row_col\n");
			goto aml_ldim_get_config_dts_next;
		}
	}
	if ((para[0] * para[1]) > LD_BLKREGNUM) {
		LDIMERR("zone row*col(%d*%d) is out of support\n",
			para[0], para[1]);
	} else {
		ldim_config.hist_row = para[0];
		ldim_config.hist_col = para[1];
	}
	LDIMPR("get region row = %d, col = %d\n",
	       ldim_config.hist_row, ldim_config.hist_col);

aml_ldim_get_config_dts_next:
	/* get bl_mode from dts */
	ret = of_property_read_u32(child, "bl_ldim_mode", &para[0]);
	if (ret)
		LDIMERR("failed to get bl_ldim_mode\n");
	else
		ldim_config.bl_mode = (unsigned char)para[0];
	LDIMPR("get bl_mode = %d\n", ldim_config.bl_mode);

	/* get ldim_dev_index from dts */
	ret = of_property_read_u32(child, "ldim_dev_index", &para[0]);
	if (ret)
		LDIMERR("failed to get ldim_dev_index\n");
	else
		ldim_config.dev_index = (unsigned char)para[0];
	if (ldim_config.dev_index < 0xff)
		LDIMPR("get ldim_dev_index = %d\n", ldim_config.dev_index);

	return 0;
}

int aml_ldim_get_config_unifykey(unsigned char *buf)
{
	unsigned char *p;
	struct vinfo_s *vinfo = get_current_vinfo();

	/* default setting */
	ldim_config.hsize = vinfo->width;
	ldim_config.vsize = vinfo->height;

	p = buf;

	/* ldim: 24byte */
	/* get bl_ldim_region_row_col 4byte*/
	ldim_config.hist_row = *(p + LCD_UKEY_BL_LDIM_ROW);
	ldim_config.hist_col = *(p + LCD_UKEY_BL_LDIM_COL);
	LDIMPR("get zone row = %d, col = %d\n",
	       ldim_config.hist_row, ldim_config.hist_col);

	/* get bl_ldim_mode 1byte*/
	ldim_config.bl_mode = *(p + LCD_UKEY_BL_LDIM_MODE);
	LDIMPR("get bl_mode = %d\n", ldim_config.bl_mode);

	/* get ldim_dev_index 1byte*/
	ldim_config.dev_index = *(p + LCD_UKEY_BL_LDIM_DEV_INDEX);
	if (ldim_config.dev_index < 0xff)
		LDIMPR("get dev_index = %d\n", ldim_config.dev_index);

	return 0;
}

static int ldim_rmem_tm2(void)
{
	/* init reserved memory */
	ldim_rmem.wr_mem_vaddr1 = kmalloc(ldim_rmem.wr_mem_size, GFP_KERNEL);
	if (!ldim_rmem.wr_mem_vaddr1)
		goto ldim_rmem_err0;
	ldim_rmem.wr_mem_paddr1 = virt_to_phys(ldim_rmem.wr_mem_vaddr1);

	ldim_rmem.wr_mem_vaddr2 = kmalloc(ldim_rmem.wr_mem_size, GFP_KERNEL);
	if (!ldim_rmem.wr_mem_vaddr2)
		goto ldim_rmem_err1;
	ldim_rmem.wr_mem_paddr2 = virt_to_phys(ldim_rmem.wr_mem_vaddr2);

	ldim_rmem.rd_mem_vaddr1 = kmalloc(ldim_rmem.rd_mem_size, GFP_KERNEL);
	if (!ldim_rmem.rd_mem_vaddr1)
		goto ldim_rmem_err2;
	ldim_rmem.rd_mem_paddr1 = virt_to_phys(ldim_rmem.rd_mem_vaddr1);

	ldim_rmem.rd_mem_vaddr2 = kmalloc(ldim_rmem.rd_mem_size, GFP_KERNEL);
	if (!ldim_rmem.rd_mem_vaddr2)
		goto ldim_rmem_err3;
	ldim_rmem.rd_mem_paddr2 = virt_to_phys(ldim_rmem.rd_mem_vaddr2);

	memset(ldim_rmem.wr_mem_vaddr1, 0, ldim_rmem.wr_mem_size);
	memset(ldim_rmem.wr_mem_vaddr2, 0, ldim_rmem.wr_mem_size);
	memset(ldim_rmem.rd_mem_vaddr1, 0, ldim_rmem.rd_mem_size);
	memset(ldim_rmem.rd_mem_vaddr2, 0, ldim_rmem.rd_mem_size);

	return 0;

ldim_rmem_err3:
	kfree(ldim_rmem.rd_mem_vaddr1);
ldim_rmem_err2:
	kfree(ldim_rmem.wr_mem_vaddr2);
ldim_rmem_err1:
	kfree(ldim_rmem.wr_mem_vaddr1);
ldim_rmem_err0:
	return -1;
}

static int aml_ldim_malloc(unsigned int hist_row, unsigned int hist_col)
{
	struct ldim_fw_para_s *fw_para = ldim_driver.fw_para;
	int ret = 0;

	if (!fw_para)
		goto ldim_malloc_err0;

	ldim_driver.hist_matrix = kcalloc((hist_row * hist_col * 16),
					  sizeof(unsigned int), GFP_KERNEL);
	if (!ldim_driver.hist_matrix)
		goto ldim_malloc_err0;

	ldim_driver.max_rgb = kcalloc((hist_row * hist_col * 3),
				      sizeof(unsigned int), GFP_KERNEL);
	if (!ldim_driver.max_rgb)
		goto ldim_malloc_err1;

	ldim_driver.test_matrix = kcalloc((hist_row * hist_col),
					   sizeof(unsigned short), GFP_KERNEL);
	if (!ldim_driver.test_matrix)
		goto ldim_malloc_err2;

	ldim_driver.local_ldim_matrix = kcalloc((hist_row * hist_col),
						sizeof(unsigned short),
						GFP_KERNEL);
	if (!ldim_driver.local_ldim_matrix)
		goto ldim_malloc_err3;

	ldim_driver.ldim_matrix_buf = kcalloc((hist_row * hist_col),
					      sizeof(unsigned short),
					      GFP_KERNEL);
	if (!ldim_driver.ldim_matrix_buf)
		goto ldim_malloc_err4;

	fw_para->fdat->SF_BL_matrix = kcalloc((hist_row * hist_col),
					       sizeof(unsigned int),
					       GFP_KERNEL);
	if (!fw_para->fdat->SF_BL_matrix)
		goto ldim_malloc_err5;

	fw_para->fdat->last_sta1_maxrgb = kcalloc((hist_row * hist_col * 3),
						  sizeof(unsigned int),
						  GFP_KERNEL);
	if (!fw_para->fdat->last_sta1_maxrgb)
		goto ldim_malloc_err6;

	fw_para->fdat->TF_BL_matrix = kcalloc((hist_row * hist_col),
					      sizeof(unsigned int),
					      GFP_KERNEL);
	if (!fw_para->fdat->TF_BL_matrix)
		goto ldim_malloc_err7;

	fw_para->fdat->TF_BL_matrix_2 = kcalloc((hist_row * hist_col),
						sizeof(unsigned int),
						GFP_KERNEL);
	if (!fw_para->fdat->TF_BL_matrix_2)
		goto ldim_malloc_err8;

	fw_para->fdat->TF_BL_alpha = kcalloc((hist_row * hist_col),
					     sizeof(unsigned int), GFP_KERNEL);
	if (!fw_para->fdat->TF_BL_alpha)
		goto ldim_malloc_err9;

	if (ldim_dev.ldim_op_func->alloc_rmem) {
		ret = ldim_dev.ldim_op_func->alloc_rmem();
		if (ret)
			goto ldim_malloc_err10;
	}

	return 0;

ldim_malloc_err10:
	kfree(ldim_driver.fw_para->fdat->TF_BL_alpha);
ldim_malloc_err9:
	kfree(ldim_driver.fw_para->fdat->TF_BL_matrix_2);
ldim_malloc_err8:
	kfree(ldim_driver.fw_para->fdat->TF_BL_matrix);
ldim_malloc_err7:
	kfree(ldim_driver.fw_para->fdat->last_sta1_maxrgb);
ldim_malloc_err6:
	kfree(ldim_driver.fw_para->fdat->SF_BL_matrix);
ldim_malloc_err5:
	kfree(ldim_driver.ldim_matrix_buf);
ldim_malloc_err4:
	kfree(ldim_driver.local_ldim_matrix);
ldim_malloc_err3:
	kfree(ldim_driver.test_matrix);
ldim_malloc_err2:
	kfree(ldim_driver.max_rgb);
ldim_malloc_err1:
	kfree(ldim_driver.hist_matrix);
ldim_malloc_err0:
	LDIMERR("%s failed\n", __func__);
	return -1;
}

static struct ldim_operate_func_s ldim_op_func_txlx = {
	.h_region_max = 24,
	.v_region_max = 16,
	.total_region_max = 384,
	.remap_update = ldim_hw_remap_update_txlx,
	.alloc_rmem = NULL,
	.stts_init = ldim_hw_stts_initial_txlx,
	.remap_init = ldim_hw_remap_init_txlx,
	.vs_arithmetic = ldim_on_vs_arithmetic,
};

static struct ldim_operate_func_s ldim_op_func_tl1 = {
	.h_region_max = 31,
	.v_region_max = 16,
	.total_region_max = 128,
	.remap_update = NULL,
	.alloc_rmem = NULL,
	.stts_init = ldim_hw_stts_initial_tl1,
	.remap_init = NULL,
	.vs_arithmetic = ldim_on_vs_arithmetic,
};

static struct ldim_operate_func_s ldim_op_func_tm2 = {
	.h_region_max = 48,
	.v_region_max = 32,
	.total_region_max = 1536,
	.remap_update = ldim_hw_remap_update_tm2,
	.alloc_rmem = ldim_rmem_tm2,
	.stts_init = ldim_hw_stts_initial_tm2,
	.remap_init = ldim_hw_remap_init_tm2,
	.vs_arithmetic = ldim_on_vs_arithmetic_tm2,
};

static int ldim_region_num_check(struct ldim_operate_func_s *ldim_func)
{
	unsigned short temp;

	if (ldim_func == NULL) {
		LDIMERR("%s: ldim_func is NULL\n", __func__);
		return -1;
	}

	if (ldim_config.hist_row > ldim_func->v_region_max) {
		LDIMERR("%s: blk row (%d) is out of support (%d)\n",
			__func__, ldim_config.hist_row,
			ldim_func->v_region_max);
		return -1;
	}
	if (ldim_config.hist_col > ldim_func->h_region_max) {
		LDIMERR("%s: blk col (%d) is out of support (%d)\n",
			__func__, ldim_config.hist_col,
			ldim_func->h_region_max);
		return -1;
	}
	temp = ldim_config.hist_row * ldim_config.hist_col;
	if (temp > ldim_func->total_region_max) {
		LDIMERR("%s: blk total region (%d) is out of support (%d)\n",
			__func__, temp, ldim_func->total_region_max);
		return -1;
	}

	return 0;
}

int aml_ldim_probe(struct platform_device *pdev)
{
	int ret = 0;
	unsigned int ldim_vsync_irq = 0;
	struct ldim_dev_s *devp = &ldim_dev;
	struct aml_bl_drv_s *bl_drv = aml_bl_get_driver();
	struct ldim_fw_para_s *fw_para = aml_ldim_get_fw_para();

	memset(devp, 0, (sizeof(struct ldim_dev_s)));

#ifdef LDIM_DEBUG_INFO
	ldim_debug_print = 1;
#endif

	ldim_level_update = 0;

	if (!fw_para) {
		LDIMERR("%s: fw_para is null\n", __func__);
		return -1;
	}
	ldim_driver.fw_para = fw_para;
	ldim_driver.fw_para->hist_row = ldim_config.hist_row;
	ldim_driver.fw_para->hist_col = ldim_config.hist_col;
	ldim_driver.fw_para->valid = 1;

	/* ldim_op_func */
	switch (bl_drv->data->chip_type) {
	case BL_CHIP_TL1:
		devp->ldim_op_func = &ldim_op_func_tl1;
		break;
	case BL_CHIP_TM2:
		devp->ldim_op_func = &ldim_op_func_tm2;
		break;
	case BL_CHIP_TXLX:
		devp->ldim_op_func = &ldim_op_func_txlx;
		break;
	default:
		devp->ldim_op_func = NULL;
		break;
	}

	ret = ldim_region_num_check(devp->ldim_op_func);
	if (ret)
		return -1;

	ret = aml_ldim_malloc(ldim_config.hist_row, ldim_config.hist_col);
	if (ret) {
		LDIMERR("%s failed\n", __func__);
		goto err;
	}

	ret = alloc_chrdev_region(&devp->aml_ldim_devno, 0, 1,
		AML_LDIM_DEVICE_NAME);
	if (ret < 0) {
		LDIMERR("failed to alloc major number\n");
		ret = -ENODEV;
		goto err;
	}

	devp->aml_ldim_clsp = class_create(THIS_MODULE, "aml_ldim");
	if (IS_ERR(devp->aml_ldim_clsp)) {
		ret = PTR_ERR(devp->aml_ldim_clsp);
		return ret;
	}
	ret = aml_ldim_debug_probe(devp->aml_ldim_clsp);
	if (ret)
		goto err1;

	/* connect the file operations with cdev */
	cdev_init(&devp->cdev, &ldim_fops);
	devp->cdev.owner = THIS_MODULE;

	/* connect the major/minor number to the cdev */
	ret = cdev_add(&devp->cdev, devp->aml_ldim_devno, 1);
	if (ret) {
		LDIMERR("%s: failed to add device\n", __func__);
		goto err2;
	}

	devp->dev = device_create(devp->aml_ldim_clsp, NULL,
		devp->aml_ldim_devno, NULL, AML_LDIM_CLASS_NAME);
	if (IS_ERR(devp->dev)) {
		ret = PTR_ERR(devp->dev);
		goto err2;
	}

	ldim_queue = create_workqueue("ldim workqueue");
	if (!ldim_queue) {
		LDIMERR("ldim_queue create failed\n");
		ret = -1;
		goto err2;
	}
	INIT_WORK(&ldim_on_vs_work, ldim_on_update_brightness);
	INIT_WORK(&ldim_off_vs_work, ldim_off_update_brightness);

	spin_lock_init(&ldim_isr_lock);
	spin_lock_init(&rdma_ldim_isr_lock);

	bl_drv->res_ldim_vsync_irq = platform_get_resource_byname
				(pdev, IORESOURCE_IRQ, "vsync");
	if (!bl_drv->res_ldim_vsync_irq) {
		ret = -ENODEV;
		LDIMERR("ldim_vsync_irq resource error\n");
		goto err2;
	}
	ldim_vsync_irq = bl_drv->res_ldim_vsync_irq->start;
	if (request_irq(ldim_vsync_irq, ldim_vsync_isr, IRQF_SHARED,
		"ldim_vsync", (void *)"ldim_vsync")) {
		LDIMERR("can't request ldim_vsync_irq(%d)\n", ldim_vsync_irq);
	}

	ldim_driver.valid_flag = 1;

	if (bl_drv->bconf->method != BL_CTRL_LOCAL_DIMMING)
		ldim_dev_add_virtual_driver(&ldim_driver);

	LDIMPR("%s ok\n", __func__);
	return 0;

err2:
	cdev_del(&devp->cdev);
err1:
	unregister_chrdev_region(devp->aml_ldim_devno, 1);
err:
	return ret;
}

int aml_ldim_remove(void)
{
	struct ldim_dev_s *devp = &ldim_dev;
	struct aml_bl_drv_s *bl_drv = aml_bl_get_driver();

	kfree(ldim_driver.fw_para->fdat->SF_BL_matrix);
	kfree(ldim_driver.fw_para->fdat->TF_BL_matrix);
	kfree(ldim_driver.fw_para->fdat->TF_BL_matrix_2);
	kfree(ldim_driver.fw_para->fdat->last_sta1_maxrgb);
	kfree(ldim_driver.fw_para->fdat->TF_BL_alpha);
	kfree(ldim_driver.ldim_matrix_buf);
	kfree(ldim_driver.hist_matrix);
	kfree(ldim_driver.max_rgb);
	kfree(ldim_driver.test_matrix);
	kfree(ldim_driver.local_ldim_matrix);

	free_irq(bl_drv->res_ldim_vsync_irq->start, (void *)"ldim_vsync");

	//cdev_del(devp->aml_ldim_cdevp);
	//kfree(devp->aml_ldim_cdevp);
	aml_ldim_debug_remove(devp->aml_ldim_clsp);
	class_destroy(devp->aml_ldim_clsp);
	unregister_chrdev_region(devp->aml_ldim_devno, 1);

	LDIMPR("%s ok\n", __func__);
	return 0;
}

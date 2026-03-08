/*
 * include/linux/amlogic/media/video_sink/vpp_afd.h
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

#ifndef VPP_AFD_
#define VPP_AFD_

enum E_AFD_RESULT {
	AFD_RESULT_ERROR = -1,
	AFD_RESULT_OK = 0,
	AFD_RESULT_ENABLE_CHANGED = 1,
	AFD_RESULT_CROP_CHANGED = 1 << 1,
	AFD_RESULT_DISP_CHANGED = 1 << 2,
	AFD_RESULT_MAX = 0xff,
};

struct ar_fraction_s {
	int numerator;
	int denominator;
};

struct crop_rect_s {
	unsigned int top;
	unsigned int left;
	unsigned int bottom;
	unsigned int right;
};

struct pos_rect_s {
	unsigned int x_start;
	unsigned int y_start;
	unsigned int x_end;
	unsigned int y_end;
};

struct afd_in_param {
	void *ud_param;
	u32 ud_param_size;
	unsigned int video_w;
	unsigned int video_h;
	unsigned int screen_w;
	unsigned int screen_h;
	struct pos_rect_s disp_info;
	struct ar_fraction_s video_ar;
	struct ar_fraction_s screen_ar;
};

struct afd_out_param {
	bool afd_enable;
	struct crop_rect_s crop_info;
	struct pos_rect_s dst_pos;
};

typedef void *(*afd_init)(void);
typedef int (*afd_info_get)(void *handle, struct afd_in_param *in,
			struct afd_out_param *out);
typedef void (*afd_uninit)(void *handle);

struct VPP_AFD_Func_Ptr {
	afd_init init;
	afd_info_get info_get;
	afd_uninit uninit;
};

int register_afd_function(struct VPP_AFD_Func_Ptr *func, const char *ver);
int unregister_afd_function(struct VPP_AFD_Func_Ptr *func);
int afd_process(u8 id, struct afd_in_param *in, struct afd_out_param *out);
bool is_afd_available(u8 id);
#endif

// SPDX-License-Identifier: (GPL-2.0+ OR MIT)

#ifndef __MESON_WDT_H
#define __MESON_WDT_H
extern void vWatchdogDisable(void);
extern void vWatchdogEnable(void);
extern void vWatchdogInit(uint32_t msec);
extern void vWatchdogDeInit(void);
extern void vWatchdogResetNow(void);
extern void vWatchdogPing(void);
extern void vWatchdogSetTimeout(uint32_t msec);
#endif

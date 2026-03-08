// SPDX-License-Identifier: (GPL-2.0+ OR MIT)

#define CONFIG_ETH_WAKEUP

extern void str_hw_init(void);
extern void str_hw_disable(void);
extern void str_power_on(int shutdown_flag);
extern void str_power_off(int shutdown_flag);

#ifdef CONFIG_ETH_WAKEUP
extern void xETHPowerEnable(void *data);
#if defined(SHINE_PROJECT) || defined(DAHLIA_PROJECT)
extern void xETHPowerGPIO(void *data);
#endif
#endif

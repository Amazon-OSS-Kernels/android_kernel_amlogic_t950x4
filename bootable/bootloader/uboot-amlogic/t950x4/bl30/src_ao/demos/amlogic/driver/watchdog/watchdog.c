
/*
 * watchdog driver
 * using the step:
 * 1. vWatchdogInit(msec);
 * 2. vWatchdogDeInit();
 */

#include "FreeRTOS.h"
#include <register.h>
#include <common.h>
#include <watchdog.h>
#include <task.h>
#include "timers.h"

#define WATCHDOG_TIMEOUT_MAX 0xffff
#define WDTDEBUG	0

#define WDTDebug(fmt, x...)						\
do {									\
	if (WDTDEBUG)						\
		iprintf("[WdtDebug]: %s: "fmt , __func__,  ##x);\
} while (0)

#ifdef WATCHDOG_CNTL
#define	WATCHDOG_REG_CTRL0	WATCHDOG_CNTL
#define	WATCHDOG_REG_CTRL1	WATCHDOG_CNTL1
#define	WATCHDOG_REG_CNT	WATCHDOG_TCNT
#define	WATCHDOG_REG_CLR	WATCHDOG_RESET
#else
#define	WATCHDOG_REG_CTRL0	RESETCTRL_WATCHDOG_CTRL0
#define	WATCHDOG_REG_CTRL1	RESETCTRL_WATCHDOG_CTRL1
#define	WATCHDOG_REG_CNT	RESETCTRL_WATCHDOG_CNT
#define	WATCHDOG_REG_CLR	RESETCTRL_WATCHDOG_CLR
#endif

static void vWatchdogTimer(TimerHandle_t xTimer);
TimerHandle_t xWdtTimer = NULL;

void vWatchdogPing(void)
{
	REG32(WATCHDOG_REG_CLR) = 0x0;
}

void vWatchdogDisable(void)
{
	vWatchdogPing();
	REG32(WATCHDOG_REG_CTRL0) &= ~(1 << 18);
}

void vWatchdogEnable(void)
{
	REG32(WATCHDOG_REG_CTRL0) |= 1 << 18;
}

void vWatchdogDeInit(void)
{
	vWatchdogPing();
	if (xWdtTimer)
		xTimerStop(xWdtTimer, 0);
	WDTDebug("Stop Watchdog\n");
}

void vWatchdogInit(uint32_t msec)
{
	WDTDebug("ctrl0 reg 0x%x, value is 0x%x\n", WATCHDOG_REG_CTRL0, REG32(WATCHDOG_REG_CTRL0));
	if (!xWdtTimer)
		xWdtTimer = xTimerCreate("wdtTimer", pdMS_TO_TICKS(msec >> 1), pdTRUE, NULL, vWatchdogTimer);
	xTimerStart(xWdtTimer, 0);
	WDTDebug("Start Watchdog\n");
}

/*
 * Restart the system immediately
 */
void vWatchdogResetNow(void)
{
	int i;

	while (1) {
#ifdef WATCHDOG_TCNT /* for t5/t5d only */
		REG32(WATCHDOG_REG_CTRL0) = ((1 << 26)	// sys_reset_n_now
					| (0 << 18));	// watchdog_en
#else
		REG32(WATCHDOG_REG_CTRL0) = ((1 << 27)	// sys_reset_n_now
					| (0 << 18));	// watchdog_en
#endif
		/* Decive GCC for waiting some cycles */
		for (i = 0; i < 100; i++)
			REG32(WATCHDOG_REG_CTRL0);
	}
}

void vWatchdogSetTimeout(uint32_t msec)
{
	if (msec > WATCHDOG_TIMEOUT_MAX) {
		WDTDebug("wdt timeout must be smaller than 0xffff!\n");
		msec = WATCHDOG_TIMEOUT_MAX;
	}
	vWatchdogPing();
	REG32(WATCHDOG_REG_CNT) = msec;
}
static void vWatchdogTimer(TimerHandle_t xTimer)
{
	xTimer = xTimer;
	WDTDebug("---------pet wdt---------\n");
	vWatchdogPing();
}

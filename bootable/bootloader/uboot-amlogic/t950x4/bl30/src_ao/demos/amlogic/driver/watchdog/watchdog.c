/*
 * Copyright (C) 2014-2018 Amlogic, Inc. All rights reserved.
 *
 * All information contained herein is Amlogic confidential.
 *
 * This software is provided to you pursuant to Software License Agreement
 * (SLA) with Amlogic Inc ("Amlogic"). This software may be used
 * only in accordance with the terms of this agreement.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification is strictly prohibited without prior written permission from
 * Amlogic.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

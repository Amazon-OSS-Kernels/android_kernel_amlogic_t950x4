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

#include "FreeRTOS.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <timers.h>
#include <task.h>
#include "keypad.h"
#include "saradc.h"

struct xOneAdcKeyInfo {
	KeyState keyState;
	int jitterCount;
	int keepCount;
	int ignoreEvent;
	struct xAdcKeyInfo *adcKeyInfo;
	struct xOneAdcKeyInfo *xNext;
};

static struct xOneAdcKeyInfo *xHeadKey;
static TimerHandle_t xAdcKeyCycleTimer;

static void prReportEvent(struct xAdcKeyInfo *xKey, uint32_t event)
{
	struct xReportEvent reportEvent;

	reportEvent.ulCode = xKey->keyInitInfo.ulKeyId;
	reportEvent.data = xKey->keyInitInfo.data;
	reportEvent.event = event;
	reportEvent.responseTime = 0;

	if (xKey->keyInitInfo.CallBack)
		xKey->keyInitInfo.CallBack(reportEvent);
}

static void prAdcKeyProcess(TimerHandle_t xTimer)
{
	int ret;
	uint16_t usAdcData;
	struct xOneAdcKeyInfo *xPassBtn = xHeadKey;
	struct xAdcKeyInfo *adcKeyInfo;

	(void)xTimer;
	for (xPassBtn = xHeadKey; xPassBtn != NULL;
						xPassBtn = xPassBtn->xNext) {
		adcKeyInfo = xPassBtn->adcKeyInfo;
#if SARADC_SUPPORT_ALWAYS_ON
                ret = xAdcGetSample(&usAdcData, 1, &(adcKeyInfo->xAdcDecp),
                                    SAMPLE_MODE_POLL);
#else
                ret = xAdcGetSample(&usAdcData, 1, &(adcKeyInfo->xAdcDecp),
                                    SAMPLE_MODE_INT);
#endif
		if (ret < 0)
			continue;
		if (xPassBtn->keyState == UP) {
			if ((usAdcData >= adcKeyInfo->ulValue -
						SAMPLE_DEVIATION)
			    && (usAdcData <=
				adcKeyInfo->ulValue + SAMPLE_DEVIATION)) {
				if (xPassBtn->jitterCount < KEY_JITTER_COUNT) {
					xPassBtn->jitterCount++;
					return;
				}
				xPassBtn->keepCount = xPassBtn->jitterCount;
				xPassBtn->jitterCount = 0;
				xPassBtn->keyState = DOWN;
			} else {
				xPassBtn->jitterCount = 0;
			}
		} else if (xPassBtn->keyState == DOWN) {
			if (((usAdcData <= adcKeyInfo->ulValue -
							SAMPLE_DEVIATION)
			     || (usAdcData >=
				 adcKeyInfo->ulValue + SAMPLE_DEVIATION))) {
				if (xPassBtn->jitterCount < KEY_JITTER_COUNT) {
					xPassBtn->jitterCount++;
					return;
				}
				if ((xPassBtn->keepCount >= THRESHOLD_SHORT)
					&& xPassBtn->ignoreEvent == 0)
					prReportEvent(adcKeyInfo, EVENT_SHORT);
				xPassBtn->jitterCount = 0;
				xPassBtn->keyState = UP;
				xPassBtn->ignoreEvent = 0;
			} else {
				xPassBtn->jitterCount = 0;
				if (xPassBtn->keepCount < THRESHOLD_LONG) {
					xPassBtn->keepCount++;
				}
				if (xPassBtn->ignoreEvent)
					continue;
				if (adcKeyInfo->keyInitInfo.eventMask & EVENT_LONG) {
					if (xPassBtn->keepCount >= THRESHOLD_LONG) {
						xPassBtn->keyState = UP;
						prReportEvent(adcKeyInfo, EVENT_LONG);
						xPassBtn->ignoreEvent = 1;
					}
				 } else if (adcKeyInfo->keyInitInfo.eventMask & EVENT_SHORT) {
					if (xPassBtn->keepCount >= THRESHOLD_SHORT) {
						xPassBtn->keyState = UP;
						prReportEvent(adcKeyInfo, EVENT_SHORT);
						xPassBtn->ignoreEvent = 1;
					}
				 }
			}
		}
	}
}

static void prvAddAdcKey(struct xOneAdcKeyInfo *xKey)
{
	if (!xKey)
		return;
	xKey->xNext = xHeadKey;
	xHeadKey = xKey;
}

void vCreateAdcKey(struct xAdcKeyInfo *keyArr, uint16_t keyNum)
{
	uint16_t i;
	struct xOneAdcKeyInfo *xOneKey;
	struct xKeyInitInfo *keyInitInfo;

	for (i = 0; i < keyNum; i++) {
		keyInitInfo = &(keyArr[i].keyInitInfo);

		if (keyInitInfo->ulKeyId < ADCKEY_ID_BASE) {
			printf("adc key:[%d] key code not less than 512.\n", i);
			continue;
		}

		xOneKey = pvPortMalloc(sizeof(struct xOneAdcKeyInfo));
		if (xOneKey == NULL) {
			printf("adc key: [%d] malloc failed!\n", i);
			continue;
		}

		memset(xOneKey, 0, sizeof(struct xOneAdcKeyInfo));
		xOneKey->keyState = UP;
		xOneKey->adcKeyInfo = &keyArr[i];
		prvAddAdcKey(xOneKey);
	}

	if (!xAdcKeyCycleTimer) {
		xAdcKeyCycleTimer = xTimerCreate((const char *)"xAdcKeyTimer",
						 pdMS_TO_TICKS
						 (TIMER_CYCLE_TIME), pdTRUE,
						 (void *)1,
						 (TimerCallbackFunction_t)
						 prAdcKeyProcess);
		if (!xAdcKeyCycleTimer) {
			printf("adc timer create failed!\n");
			return;
		}
	}
}

void vDestoryAdcKey(void)
{
	struct xOneAdcKeyInfo *xPassBtn, *xTmpBtn;

	for (xPassBtn = xHeadKey; xPassBtn != NULL;) {
		xTmpBtn = xPassBtn;
		xPassBtn = xPassBtn->xNext;

		vPortFree(xTmpBtn);
	}

	xHeadKey = NULL;
}

void vAdcKeyEnable(void)
{
#if SARADC_SUPPORT_ALWAYS_ON
	vAdcInit(SAMPLE_MODE_POLL);
#else
	vAdcInit(SAMPLE_MODE_INT);
#endif
	vAdcHwEnable();

	if (xAdcKeyCycleTimer) {
		xTimerStart(xAdcKeyCycleTimer, 0);
	}
}

void vAdcKeyDisable(void)
{
	if (xAdcKeyCycleTimer) {
		xTimerStop(xAdcKeyCycleTimer, pdMS_TO_TICKS(TIMER_CYCLE_TIME));
	}

	vAdcHwDisable();
#if SARADC_SUPPORT_ALWAYS_ON
	vAdcDeinit(SAMPLE_MODE_POLL);
#else
	vAdcDeinit(SAMPLE_MODE_INT);
#endif
}

// SPDX-License-Identifier: (GPL-2.0+ OR MIT)

#ifndef __MESON_Button_H
#define __MESON_Button_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include <timers.h>
#include <task.h>
#include <gpio.h>
#include <saradc.h>

/*common macro*/
#if SARADC_SUPPORT_ALWAYS_ON
#define TIMER_CYCLE_TIME		50
#else
#define TIMER_CYCLE_TIME		20
#endif

#define KEY_JITTER_COUNT		1

/* adc key param */
#define SAMPLE_DEVIATION		80
#define ADCKEY_ID_BASE			512

/* key event */
#define EVENT_SHORT			(1 << 0)
#define EVENT_LONG			(1 << 1)

/* key threshold */
#define THRESHOLD_SHORT			KEY_JITTER_COUNT
#define THRESHOLD_LONG			(10000/TIMER_CYCLE_TIME)

typedef enum {
	GPIO_KEY = 0,
	ADC_KEY,
	KEY_TYPE_NUM
} KeyType;

typedef enum {
	LOW = 0,
	HIGH
} GpioLevel;

typedef enum {
	UP = 0,
	DOWN,
} KeyState;

struct xReportEvent {
	uint32_t ulCode;
	uint32_t event;
	TickType_t responseTime;
	void *data;
};

struct xKeyInitInfo {
	uint32_t ulKeyId;
	uint32_t eventMask;
	uint32_t repeatTimeMs;
	uint32_t repeatDelayTimeMs;
	uint32_t longDTTMs;
	uint32_t doubleDTTMs;
	uint32_t combLongDTTMs;
	uint32_t combDTTMs;
	void (*CallBack)(struct xReportEvent);
	void *data;
};

struct xGpioKeyInfo {
	GpioLevel ulInitLevel;
	struct xKeyInitInfo keyInitInfo;
};

struct xAdcKeyInfo {
	int32_t ulValue;
	AdcInstanceConfig_t xAdcDecp;
	struct xKeyInitInfo keyInitInfo;
};

#define KEY_INIT_INFO(_ulKeyId, _eventMask,	_repeatTimeMs,\
		      _repeatDelayTimeMs, _longDTTMs, _doubleDTTMs,\
		      _combLongDTTMs, _combDTTMs, _CallBack, _data) {	\
	.ulKeyId = _ulKeyId,		\
	.eventMask = _eventMask,		\
	.repeatTimeMs = _repeatTimeMs,		\
	.repeatDelayTimeMs = _repeatDelayTimeMs,	\
	.longDTTMs = _longDTTMs,		\
	.doubleDTTMs = _doubleDTTMs,	\
	.combLongDTTMs = _combLongDTTMs,		\
	.combDTTMs = _combDTTMs,		\
	.CallBack = _CallBack,		\
	.data = _data,			\
}

#define GPIO_KEY_INFO(_ulKeyId, _ulInitLevel, _eventMask,\
		      _CallBack, _data) {	\
	.ulInitLevel = _ulInitLevel,		\
	.keyInitInfo = KEY_INIT_INFO(_ulKeyId, _eventMask, 0, 0, 0,\
				     0, 0, 0, _CallBack, _data),\
}

#define ADC_KEY_INFO(_ulKeyId, _ulValue, _adcChan, _eventMask,\
		     _CallBack, _data) {	\
	.ulValue = _ulValue,			\
	.xAdcDecp = {_adcChan, NO_AVERAGING, 1},			\
	.keyInitInfo = KEY_INIT_INFO(_ulKeyId, _eventMask, 0, 0, 0,\
				     0, 0, 0, _CallBack, _data),\
}

void vCreateGpioKey(struct xGpioKeyInfo *keyArr, uint16_t keyNum);
void vDestoryGpioKey(void);
void vGpioKeyEnable(void);
void vGpioKeyDisable(void);

void vCreateAdcKey(struct xAdcKeyInfo *keyArr, uint16_t keyNum);
void vDestoryAdcKey(void);
void vAdcKeyEnable(void);
void vAdcKeyDisable(void);

void vKeyPadInit(void);
void vKeyPadDeinit(void);

#ifdef __cplusplus
}
#endif
#endif

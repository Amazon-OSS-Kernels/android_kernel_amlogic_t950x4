#include <string.h>
#include "FreeRTOS.h"
#include "suspend.h"
#include "task.h"
#include "gpio.h"

#include "queue.h"    /* RTOS queue related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */
#include "semphr.h"   /* Semaphore related API prototypes. */

#define WIFI_WAKE_HOST GPIOB_12  //wifi_wake_host pin
#define INFO(fmt, args...) printf("[%s] " fmt "\n", __func__, ##args)

void Wifi_IRQHandle(void);
void Wifi_GpioIRQRegister(void);
void Wifi_GpioIRQFree(void);

void Wifi_IRQHandle(void)
{
	uint32_t buf[4] = {0};
	INFO("wifi resume");
	vDisableGpioIRQ(WIFI_WAKE_HOST);
	if (!xGpioGetValue(WIFI_WAKE_HOST)) {
		buf[0] = WIFI_WAKEUP;
		STR_Wakeup_src_Queue_Send_FromISR(buf);
	}
}

void Wifi_GpioIRQRegister(void)
{
	INFO();
	xPinmuxSet(GPIOB_12, PIN_FUNC0);
	xPinconfSet(WIFI_WAKE_HOST, PINF_CONFIG_BIAS_PULL_UP);
	xGpioSetDir(WIFI_WAKE_HOST, GPIO_DIR_IN);
	xRequestGpioIRQ(WIFI_WAKE_HOST, Wifi_IRQHandle, IRQF_TRIGGER_FALLING);
}

void Wifi_GpioIRQFree(void)
{
	vFreeGpioIRQ(WIFI_WAKE_HOST);
}



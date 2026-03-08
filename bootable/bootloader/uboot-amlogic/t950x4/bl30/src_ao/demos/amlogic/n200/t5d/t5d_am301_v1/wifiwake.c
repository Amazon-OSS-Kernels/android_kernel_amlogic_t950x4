#include <string.h>
#include "FreeRTOS.h"
#include "suspend.h"
#include "task.h"
#include "gpio.h"

#include "queue.h"    /* RTOS queue related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */
#include "semphr.h"   /* Semaphore related API prototypes. */

#ifdef DAHLIA_PROJECT
#define WIFI_WAKE_HOST GPIOB_13  //wifi_wake_host pin
#else
#define WIFI_WAKE_HOST GPIOB_12  //wifi_wake_host pin
#endif

#define INFO(fmt, args...) printf("[%s] " fmt "\n", __func__, ##args)

#ifdef SHINE_PROJECT
uint32_t wake_up_pin = 76;
int led_brightness = 20;
int poweroff_gpioh7 = 0;
int led_flag = 0;
int RC5_flag = 0;
#endif

#ifdef DAHLIA_PROJECT
uint32_t hwid_ch2 = 0;
#endif

void Wifi_IRQHandle(void);
void Wifi_GpioIRQRegister(void);
void Wifi_GpioIRQFree(void);

#ifdef SHINE_PROJECT
void Wifi_IRQHandle(void)
{
	uint32_t buf[4] = {0};
	INFO("wifi resume");
	vDisableGpioIRQ(wake_up_pin);
	if (!xGpioGetValue(wake_up_pin)) {
		buf[0] = WIFI_WAKEUP;
		STR_Wakeup_src_Queue_Send_FromISR(buf);
	}
}

void Wifi_GpioIRQRegister(void)
{
	INFO();
	xPinmuxSet(wake_up_pin, PIN_FUNC0);
	xPinconfSet(wake_up_pin, PINF_CONFIG_BIAS_PULL_UP);
	xGpioSetDir(wake_up_pin, GPIO_DIR_IN);
	xRequestGpioIRQ(wake_up_pin, Wifi_IRQHandle, IRQF_TRIGGER_FALLING);
}

void Wifi_GpioIRQFree(void)
{
	vFreeGpioIRQ(wake_up_pin);
}

void xETHPowerGPIO(void *data)
{
	if(*(u32 *)data){
		wake_up_pin = *(u32 *)data;
	}
	if(*(((u32 *)data) + 2)){
		led_brightness = *(((u32 *)data) + 2);
	}
	if(*(((u32 *)data) + 1)){
		led_flag = *(((u32 *)data) + 1);
	}
	poweroff_gpioh7 = *(((u32 *)data) + 3);
	RC5_flag = *(((u32 *)data) + 4);
        iprintf("xETHPowerGPIO: -wol_wake_pin=%d=======\n",*(u32 *)data);
        iprintf("xETHPowerGPIO: -led_flags=%d=======\n",*(((u32 *)data) + 1));
        iprintf("xETHPowerGPIO: -led_brightness=%d=======\n",*(((u32 *)data) + 2));
	iprintf("xETHPowerGPIO: -power off GPIOH_7=%d=======\n",*(((u32 *)data) + 3));
	iprintf("xETHPowerGPIO: -RC5_flag=%d=======\n",*(((u32 *)data) + 4));
}
#else

#ifdef DAHLIA_PROJECT
void xETHPowerGPIO(void *data)
{
        if(*(u32 *)data){
                 hwid_ch2 = *(u32 *)data;
        }
        iprintf("xETHPowerGPIO: -hwid-ch2=%d=======\n",*(u32 *)data);
}
#endif
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
#endif

#include <string.h>
#include "FreeRTOS.h"
#include "ir.h"
#include "keypad.h"
#include "gpio.h"
#include "saradc.h"
#include "suspend.h"


#define ADC_KEY_ID_HOME		520

#ifdef WATCHDOG_CNTL
#define        WATCHDOG_REG_CTRL0      WATCHDOG_CNTL
#define        WATCHDOG_REG_CTRL1      WATCHDOG_CNTL1
#define        WATCHDOG_REG_CNT        WATCHDOG_TCNT
#define        WATCHDOG_REG_CLR        WATCHDOG_RESET
#else
#define        WATCHDOG_REG_CTRL0      RESETCTRL_WATCHDOG_CTRL0
#define        WATCHDOG_REG_CTRL1      RESETCTRL_WATCHDOG_CTRL1
#define        WATCHDOG_REG_CNT        RESETCTRL_WATCHDOG_CNT
#define        WATCHDOG_REG_CLR        RESETCTRL_WATCHDOG_CLR
#endif

extern uint32_t suspend_flag;

static inline void vWatchdogResetNow(void)
{
       int i;

       while (1) {
#ifdef WATCHDOG_TCNT /* for t5/t5d only */
               REG32(WATCHDOG_REG_CTRL0) = ((1 << 26)  // sys_reset_n_now
                                       | (0 << 18));   // watchdog_en
#else
               REG32(WATCHDOG_REG_CTRL0) = ((1 << 22)  // sys_reset_n_now
                                       | (0 << 18));   // watchdog_en
#endif
               /* Decive GCC for waiting some cycles */
               for (i = 0; i < 100; i++)
                       REG32(WATCHDOG_REG_CTRL0);
       }
}

static void vAdcKeyCallBack(struct xReportEvent event)
{
	uint32_t buf[4] = {0};
	uint32_t val = 0;

	switch (event.ulCode) {
	case ADC_KEY_ID_HOME:
                if ((suspend_flag == 1) && (event.event == EVENT_SHORT)) {
			printf("ADC key: power key wakeup\n");
                        buf[0] = POWER_KEY_WAKEUP;
                        STR_Wakeup_src_Queue_Send(buf);
                } else if (event.event == EVENT_LONG){
                        printf("ADC key: watchdog reset\n");
                        /*write reboot mode is POWER_LONG_PRESS mode: 0xf*/
                        val = REG32(AO_RTI_STATUS_REG3);
                        val = (val & (~0xf)) | 0xf;
                        REG32(AO_RTI_STATUS_REG3) = val;
                        vWatchdogResetNow();
                }
		break;
	default:
		break;
	}
}



struct xAdcKeyInfo adcKeyInfo[] = {
	ADC_KEY_INFO(ADC_KEY_ID_HOME, 0, SARADC_CH1,
		     EVENT_SHORT| EVENT_LONG,
		     vAdcKeyCallBack, NULL)
};

void vKeyPadInit(void)
{
	vCreateAdcKey(adcKeyInfo,
			sizeof(adcKeyInfo)/sizeof(struct xAdcKeyInfo));
//	vGpioKeyEnable();
	vAdcKeyEnable();
}

void vKeyPadDeinit(void)
{
	vAdcKeyDisable();
//	vGpioKeyDisable();
	vDestoryAdcKey();
}

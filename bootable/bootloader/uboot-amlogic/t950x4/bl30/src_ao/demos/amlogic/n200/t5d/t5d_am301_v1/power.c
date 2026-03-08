// SPDX-License-Identifier: (GPL-2.0+ OR MIT)

#include "FreeRTOS.h"
#include "common.h"
#include "gpio.h"
#include "ir.h"
#include "suspend.h"
#include "task.h"
#include "pwm.h"
#include "pwm_plat.h"
#include "keypad.h"

#include "hdmi_cec.h"

#define CONFIG_ETH_WAKEUP
#include "watchdog.h"


#ifdef CONFIG_ETH_WAKEUP
#include "interrupt_control.h"
int eth_deinit = 0;
#define IRQ_ETH_PMT_NUM 73

uint32_t g_eth_power_enable = 0;
#endif

#ifdef DAHLIA_PROJECT
extern int hwid_ch2;
#endif

static TaskHandle_t cecTask = NULL;
static int vdd_ee;
static int vdd_cpu;

static IRPowerKey_t prvPowerKeyList[] = {
	{ 0xef10fe01, IR_NORMAL}, /* ref tv pwr */
	{ 0xba45bd02, IR_NORMAL}, /* small ir pwr */
	{ 0xef10fb04, IR_NORMAL}, /* old ref tv pwr */
	{ 0xf20dfe01, IR_NORMAL},
	{ 0xe51afb04, IR_NORMAL},
	{ 0x3ac5bd02, IR_CUSTOM},
	{ 0xb9467d02, IR_NORMAL}, /* ABC power key */
	{ 0xa05f7d02, IR_CUSTOM_1}, /* ABC netflix key */
	{ 0x5ea17d02, IR_CUSTOM_2}, /* ABC prime video key */
	{ 0x609f7d02, IR_NORMAL}, /* ABC home key */
	{ 0xb54a7d02, IR_NORMAL}, /* ABC enter key */
	{ 0x5fa07d02, IR_NORMAL}, /* ABC voice search key */
	{ 0x5da27d02, IR_CUSTOM_3}, /* ABC partner1 key */
	{ 0x5ca37d02, IR_CUSTOM_4}, /* ABC partner2 key */
        { 0x5ba47d02, IR_CUSTOM_5}, /* ABS presetting1 key */
        { 0x5aa57d02, IR_CUSTOM_6}, /* ABS presetting2 key */
	{ 0xf00f0586, IR_NORMAL}, /* Insignia remote --- power */
	{ 0x9e610586, IR_NORMAL}, /* Insignia additional remote --- power */
#if defined(SHINE_PROJECT) || defined (DAHLIA_PROJECT) || defined (HADRIAN_PROJECT)
	{ 0xf2a0d5, IR_NORMAL}, /* TCL factory remote --- power */
#endif
	{}
        /* add more */
};

static void vIRHandler(IRPowerKey_t *pkey)
{
	uint32_t buf[4] = {0};
	if (pkey->type == IR_NORMAL)
		buf[0] = REMOTE_WAKEUP;
	else if (pkey->type == IR_CUSTOM)
		buf[0] = REMOTE_CUS_WAKEUP;
	else if (pkey->type == IR_CUSTOM_1)
		buf[0] = REMOTE_CUS1_WAKEUP;
	else if (pkey->type == IR_CUSTOM_2)
		buf[0] = REMOTE_CUS2_WAKEUP;
	else if (pkey->type == IR_CUSTOM_3)
		buf[0] = REMOTE_CUS3_WAKEUP;
	else if (pkey->type == IR_CUSTOM_4)
		buf[0] = REMOTE_CUS_WAKEUP;
	else if (pkey->type == IR_CUSTOM_5)
		buf[0] = REMOTE_CUS5_WAKEUP;
        else if (pkey->type == IR_CUSTOM_6)
		buf[0] = REMOTE_CUS6_WAKEUP;


        /* do sth below  to wakeup*/
	STR_Wakeup_src_Queue_Send_FromISR(buf);
};

#ifdef CONFIG_ETH_WAKEUP
void vETHInit(uint32_t ulIrq,function_ptr_t handler);
void vETHDeint(uint32_t ulIrq);
void eth_handler(void);
#endif
void str_hw_init(void);
void str_hw_disable(void);
void str_power_on(int shutdown_flag);
void str_power_off(int shutdown_flag);
void Wifi_GpioIRQRegister(void);
void Wifi_GpioIRQFree(void);

void str_hw_init(void)
{
	/*enable device & wakeup source interrupt*/
#if defined(SHINE_PROJECT) || defined (DAHLIA_PROJECT) || defined (HADRIAN_PROJECT)
	vIRInit(MODE_HARD_RCA_NEC, GPIOD_5, PIN_FUNC1, prvPowerKeyList, ARRAY_SIZE(prvPowerKeyList), vIRHandler);
#else
	vIRInit(MODE_HARD_NEC, GPIOD_5, PIN_FUNC1, prvPowerKeyList, ARRAY_SIZE(prvPowerKeyList), vIRHandler);
#endif
#ifdef CONFIG_ETH_WAKEUP
	vETHInit(IRQ_ETH_PMT_NUM,eth_handler);
#endif
	xTaskCreate(vCEC_task, "CECtask", configMINIMAL_STACK_SIZE,
		    NULL, CEC_TASK_PRI, &cecTask);
	vBackupAndClearGpioIrqReg();
	//vKeyPadInit();
	vGpioKeyEnable();
	vGpioIRQInit();
	Wifi_GpioIRQRegister();
	return;
}


void str_hw_disable(void)
{
	/*disable wakeup source interrupt*/
	vIRDeint();
#ifdef CONFIG_ETH_WAKEUP
	vETHDeint(IRQ_ETH_PMT_NUM);
#endif
	if (cecTask) {
		vTaskDelete(cecTask);
		cec_req_irq(0);
	}
	Wifi_GpioIRQFree();
	//vKeyPadDeinit();
	vGpioKeyDisable();
	vRestoreGpioIrqReg();
	return;
}

void str_power_on(int shutdown_flag)
{
	int ret;

	/***set vdd_ee val***/
	ret = vPwmMesonsetvoltage(VDDEE_VOLT,vdd_ee);
	if (ret < 0) {
		printf("vdd_EE pwm set fail\n");
		return;
	}

	/***set vdd_ee val***/
	ret = vPwmMesonsetvoltage(VDDCPU_VOLT,vdd_cpu);
	if (ret < 0) {
		printf("vdd_CPU pwm set fail\n");
		return;
	}

	/***power on vcc3.3***/
	ret = xGpioSetDir(GPIOD_10,GPIO_DIR_OUT);
	if (ret < 0) {
		printf("vcc3.3 set gpio dir fail\n");
		return;
	}

	ret = xGpioSetValue(GPIOD_10,GPIO_LEVEL_HIGH);
	if (ret < 0) {
		printf("vcc3.3 set gpio val fail\n");
		return;
	}

	if (shutdown_flag) {
		/***power on VDDQ/VDDCPU***/
		ret = xGpioSetDir(GPIOD_4,GPIO_DIR_OUT);
		if (ret < 0) {
			printf("VDDCPU/VDDQ set gpio dir fail\n");
			return;
		}

		ret = xGpioSetValue(GPIOD_4,GPIO_LEVEL_HIGH);
		if (ret < 0) {
			printf("VDDCPU/VDDQ set gpio val fail\n");
			return;
		}
		/*Wait 200ms for VDDCPU statble*/
		vTaskDelay(pdMS_TO_TICKS(200));
	}
#ifdef DAHLIA_PROJECT
	if (hwid_ch2 == 5){
		ret = xGpioSetDir(GPIOD_3,GPIO_DIR_OUT);
        	if (ret < 0) {
                	printf("ETH power set gpio dir fail\n");
                	return;
        	}

        	ret = xGpioSetValue(GPIOD_3,GPIO_LEVEL_HIGH);
        	if (ret < 0) {
                	printf("ETH power set gpio val fail\n");
                	return;
        	}
	}
#endif
	/***power on 5v***/
	REG32(AO_GPIO_TEST_N) = REG32(AO_GPIO_TEST_N) | (1 << 31);
	vWatchdogDeInit();
}

void str_power_off(int shutdown_flag)
{
	int ret;
	vWatchdogInit(6000);

	printf("poweroff 5v\n");
	printf("0x%x\n", REG32(AO_GPIO_TEST_N));

#ifdef CONFIG_ETH_WAKEUP
	if ( 0 == g_eth_power_enable ){
		REG32(AO_GPIO_TEST_N) = (REG32(AO_GPIO_TEST_N) << 1) >> 1;
#ifdef DAHLIA_PROJECT
		if (hwid_ch2 == 5){
			ret = xGpioSetDir(GPIOD_3,GPIO_DIR_OUT);
                	if (ret < 0) {
                        	printf("ETH set gpio dir fail\n");
                        	return;
                	}

                	ret = xGpioSetValue(GPIOD_3,GPIO_LEVEL_LOW);
                	if (ret < 0) {
                        	printf("ETH set gpio val fail\n");
                        	return;
                	}
		}
#endif
	}
#else
	REG32(AO_GPIO_TEST_N) = (REG32(AO_GPIO_TEST_N) << 1) >> 1;
#endif

#if defined (SHINE_PROJECT) || defined (HADRIAN_PROJECT)
	if (0)
#else
	if (shutdown_flag)
#endif
	{
		/***power off VDDQ/VDDCPU***/
		ret = xGpioSetDir(GPIOD_4,GPIO_DIR_OUT);
		if (ret < 0) {
			printf("VDDCPU/VDDQ set gpio dir fail\n");
			return;
		}

		ret = xGpioSetValue(GPIOD_4,GPIO_LEVEL_LOW);
		if (ret < 0) {
			printf("VDDCPU/VDDQ set gpio val fail\n");
			return;
		}
	}

	/***power off vcc3.3***/
	ret = xGpioSetDir(GPIOD_10,GPIO_DIR_OUT);
	if (ret < 0) {
		printf("vcc3.3 set gpio dir fail\n");
		return;
	}

	ret= xGpioSetValue(GPIOD_10,GPIO_LEVEL_LOW);
	if (ret < 0) {
		printf("vcc3.3 set gpio val fail\n");
		return;
	}

	/***set vdd_cpu val***/
	vdd_cpu = vPwmMesongetvoltage(VDDCPU_VOLT);
	if (vdd_cpu < 0) {
		printf("vdd_CPU pwm get fail\n");
		return;
	}

	ret = vPwmMesonsetvoltage(VDDCPU_VOLT,700);
	if (ret < 0) {
		printf("vdd_CPU pwm set fail\n");
		return;
	}

	/***set vdd_ee val***/
	vdd_ee = vPwmMesongetvoltage(VDDEE_VOLT);
	if (vdd_ee < 0) {
		printf("vdd_EE pwm get fail\n");
		return;
	}

	ret = vPwmMesonsetvoltage(VDDEE_VOLT,770);
	if (ret < 0) {
		printf("vdd_EE pwm set fail\n");
		return;
	}

	//REG32( ((0x0000 << 2) + 0xff638c00)) = 0;
}

#ifdef CONFIG_ETH_WAKEUP
void eth_handler(void)
{
	uint32_t buf[4] = {0};
	if (eth_deinit == 0) {
		buf[0] = ETH_PMT_WAKEUP;
		STR_Wakeup_src_Queue_Send_FromISR(buf);
		DisableIrq(IRQ_ETH_PMT_NUM);
	} else {
		eth_deinit = 0;
	}
}

void vETHInit(uint32_t ulIrq,function_ptr_t handler)
{
	RegisterIrq(ulIrq, 2, handler);
//	EnableIrq(ulIrq);
}

void vETHDeint(uint32_t ulIrq)
{
	eth_deinit = 1;
	DisableIrq(ulIrq);
	UnRegisterIrq(ulIrq);
}

void xETHPowerEnable(void *data)
{
	g_eth_power_enable = *((uint32_t *)data);
	printf("xETHPowerEnable: %d\n",g_eth_power_enable);
}
#endif

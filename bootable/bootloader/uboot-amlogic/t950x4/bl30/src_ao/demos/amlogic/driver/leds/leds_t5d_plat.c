// SPDX-License-Identifier: (GPL-2.0+ OR MIT)


#include "FreeRTOS.h"
#include <gpio.h>
#include <leds_plat.h>
#include <leds_state.h>

/* TODO: Temporarily use static variables instead of stick mem */
static int32_t LedStickMem0;
#ifdef SHINE_PROJECT
extern int led_max;
static int32_t LedStickMem1;
#endif


static LedCoord_t BreathInflections0[] = {
	{0, 0},
	{2500, 255},
	{5000, 0}
};

static LedCoord_t BreathInflections1[] = {
	{0, 0},
	{5000, 255},
	{10000, 0}
};

static LedCoord_t BreathInflections2[] = {
	{0, 0},
	{10000, 255},
	{20000, 0}
};

static LedCoord_t BreathInflections3[] = {
	{0, 0},
	{20000, 255},
	{40000, 0}
};


// FOR AMAZON LED Breath
static LedCoord_t BreathInflections_amazon[]= {
    {0, 0},
    {200, 25},
    {400, 50},
    {600, 75},
    {800, 100},
    {1000, 125},
    {1200, 150},
    {1400, 175},
    {1600, 200},
    {1800, 225},
    {2300, 255},
    {2700, 200},
    {3100, 150},
    {3500, 100},
    {3700, 50},
    {3900, 0},
};


LedCoord_t *BreathInflections[LED_BREATH_MAX_COUNT] = {
	BreathInflections0,
	BreathInflections1,
	BreathInflections2,
	BreathInflections3,
	BreathInflections_amazon,
};

#ifdef DAHLIA_PROJECT
LedDevice_t MesonLeds[] = {
        {
                .id = LED_ID_0,
                .type = LED_TYPE_PWM,
                .name = "sys_led",
                .hardware_id = LED_PWM_C,
                .polarity = LED_POLARITY_POSITIVE,  // AMAZON PWM is invert
                .breathtime = 0,
        },
};
#elif SHINE_PROJECT
LedDevice_t MesonLeds[] = {
        {
                .id = LED_ID_0,           //green
                .type = LED_TYPE_PWM,
                .name = "sys_led",
                .hardware_id = LED_PWM_C,
                .polarity = LED_POLARITY_INVERT,  // AMAZON PWM is invert
                .breathtime = 0,
        },
	{
		.id = LED_ID_1,           //vestel dual-color red led
		.type = LED_TYPE_PWM,
		.name = "sys_led2",
		.hardware_id = LED_PWM_D,
		.polarity = LED_POLARITY_INVERT,
		.breathtime = 0,
	},      
};
#else
LedDevice_t MesonLeds[] = {
	{
		.id = LED_ID_0,
		.type = LED_TYPE_PWM,
		.name = "sys_led",
		.hardware_id = LED_PWM_C,
		.polarity = LED_POLARITY_INVERT,  // AMAZON PWM is invert
		.breathtime = 0,
	},
};
#endif

int32_t get_led_breath_len(uint32_t breath_id)
{
	switch (breath_id) {
		case 0:
			return sizeof(BreathInflections0)/sizeof(LedCoord_t);
		case 1:
			return sizeof(BreathInflections1)/sizeof(LedCoord_t);
		case 2:
			return sizeof(BreathInflections2)/sizeof(LedCoord_t);
		case 3:
			return sizeof(BreathInflections3)/sizeof(LedCoord_t);
		case 4:
			return sizeof(BreathInflections_amazon)/sizeof(LedCoord_t);
	} /* end swith */

	iprintf("%s: id: %ld leds state init!\n", DRIVER_NAME, breath_id);

	return -pdFREERTOS_ERRNO_EINVAL;
}

int32_t vLedPlatInit(int32_t ** stickmem)
{
	/* TODO: Here is initialization stickmem, but not doing so now */
	*stickmem = &LedStickMem0;
#ifdef SHINE_PROJECT
    if (led_max == 2)
	*(stickmem+1) = &LedStickMem1;
#endif
	/* off by default */
	return xLedsStateSetBrightness(LED_ID_0, LED_OFF);
}

int32_t vLedPinmuxInit(void)
{
	/* set pinmux */
	xPinmuxSet(GPIOD_7, PIN_FUNC2);
#ifdef SHINE_PROJECT
	if (led_max == 2) {
		xPinmuxSet(GPIOH_5, PIN_FUNC4);
		iprintf("%s: id: ABC dual leds state init!\n", DRIVER_NAME);
	}
#endif
	return 0;
}


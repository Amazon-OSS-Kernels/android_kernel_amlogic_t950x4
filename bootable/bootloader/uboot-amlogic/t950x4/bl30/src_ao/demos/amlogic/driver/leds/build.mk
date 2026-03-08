ifeq ($(TARGET_PRODUCT), dahlia)
CFLAGS += -DDAHLIA_PROJECT
endif
leds-y = leds_state.o leds_${SOC}_plat.o

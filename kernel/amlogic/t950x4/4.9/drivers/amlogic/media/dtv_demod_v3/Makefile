
obj-$(CONFIG_AMLOGIC_DTV_DEMOD_V3) += dtvdemod.o
#obj-y += dtvdemod.o
#obj-m += dtvdemod.o

dtvdemod-objs := demod_func.o  dvbc_func.o i2c_func.o tuner_func.o atsc_func.o dvbc_v2.o dvbc_v3.o dtmb_func.o dvbt_v2.o#dvbt_func.o

dtvdemod-objs += amlfrontend.o demod_dbg.o

dtvdemod-objs += aml_demod.o dvbs.o dvbs_diseqc.o dvbs_singlecable.o isdbt_func.o

dtvdemod-objs += lnb_controller/gpio/gpio_lnbc.o lnb_controller/wt20_1811/wt20_1811.o
dtvdemod-objs += lnb_controller/gpio/gpio_lnbc.o lnb_controller/tps65235/tps65235.o

ccflags-y += -I.
ccflags-y += -I$(srctree)/drivers/media/dvb-core
#ccflags-y += -I$(srctree)/drivers/iio/adc
ccflags-y += -I$(srctree)/include/media
ccflags-y += -Idrivers/amlogic/media/dtv_demod_v3/include
ccflags-y += -Idrivers/amlogic/media/dtv_demod_v3/lnb_controller
ccflags-y += -Idrivers/amlogic/media/dtv_demod_v3/lnb_controller/wt20_1811
ccflags-y += -I$(srctree)/drivers/amlogic/media/dtv_demod_v3/lnb_controller/tps65235
ccflags-y += -Idrivers/amlogic/media/dtv_demod_v3/lnb_controller/gpio
ccflags-y += -DDVB_CORE_ORI=1

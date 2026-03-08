
/*
 *
 * Copyright (C) 2015 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <environment.h>
#include <fdt_support.h>
#include <libfdt.h>
#include <asm/cpu_id.h>
#include <asm/arch/secure_apb.h>
#ifdef CONFIG_SYS_I2C_MESON
#include <amlogic/i2c.h>
#endif
#ifdef CONFIG_PWM_MESON
#include <pwm.h>
#include <amlogic/pwm.h>
#endif
#include <dm.h>
#ifdef CONFIG_AML_VPU
#include <vpu.h>
#endif
#include <vpp.h>
#include <amlogic/aml_v2_burning.h>
#include <amlogic/aml_v3_burning.h>
#ifdef CONFIG_AML_HDMITX20
#include <amlogic/hdmi.h>
#endif
#ifdef CONFIG_AML_LCD
#include <amlogic/aml_lcd.h>
#endif
#include <asm/arch/eth_setup.h>
#include <phy.h>
#include <linux/mtd/partitions.h>
#include <linux/sizes.h>
#include <asm-generic/gpio.h>
#include <dm.h>
#ifdef CONFIG_AML_SPIFC
#include <amlogic/spifc.h>
#endif
#ifdef CONFIG_AML_SPICC
#include <amlogic/spicc.h>
#endif
#include <asm/arch/timer.h>

//add amzn start
#ifdef CONFIG_IDME
#include <idme.h>
#endif
#if defined(UFBL_FEATURE_ONETIME_UNLOCK)
#include <amzn_onetime_unlock.h>
#endif
#if defined(UFBL_FEATURE_TEMP_UNLOCK)
#include <amzn_temp_unlock.h>
#endif
//add amzn end

DECLARE_GLOBAL_DATA_PTR;

//new static eth setup
struct eth_board_socket*  eth_board_skt;

#define CONFIG_MMC_BLOCK_SIZE     512
#define CRI_RESERVE_NUM_OF_EMMC_BLOCKS   1024  /* reserve 512k for WB and Gamma; (512k)/512*/
#define CRI_CONFIG_NUM_OF_EMMC_BLOCKS    7168  /* reserve 512k for WB and Gamma; (4M -512k)/512*/

#define CRI_CONFIG_SIZE          (CRI_CONFIG_NUM_OF_EMMC_BLOCKS * CONFIG_MMC_BLOCK_SIZE)
#define CRI_CONFIG_OFFSET        (CRI_RESERVE_NUM_OF_EMMC_BLOCKS * CONFIG_MMC_BLOCK_SIZE)

#define CRI_DATA_TCON_B0_SPI_OFFSET         (CRI_CONFIG_OFFSET + CRI_CONFIG_SIZE)
#define CRI_DATA_TCON_B0_SPI_SIZE           (CONFIG_MMC_BLOCK_SIZE * 2)
#define CRI_DATA_TCON_DEMURA_SET_OFFSET		(CRI_DATA_TCON_B0_SPI_OFFSET + CRI_DATA_TCON_B0_SPI_SIZE)
#define CRI_DATA_TCON_DEMURA_SET_SIZE       (CONFIG_MMC_BLOCK_SIZE * 2)
#define CRI_DATA_TCON_DEMURA_LUT_OFFSET     (CRI_DATA_TCON_DEMURA_SET_OFFSET + CRI_DATA_TCON_DEMURA_SET_SIZE)
#define CRI_DATA_TCON_DEMURA_LUT_SIZE       (CONFIG_MMC_BLOCK_SIZE * 512*2)
#define CRI_DATA_TCON_DEMURA_CRC_OFFSET     (CRI_DATA_TCON_DEMURA_LUT_OFFSET + CRI_DATA_TCON_DEMURA_LUT_SIZE)
#define CRI_DATA_TCON_DEMURA_CRC_SIZE       (CONFIG_MMC_BLOCK_SIZE)


enum tcon_bin_id_t {
	TCON_B0_SPI = 1,
	TCON_DEMURA_SET,
	TCON_DEMURA_LUT,
	TCON_DEMURA_CRC,
	TCON_BIN_MAX,
};

struct tcon_bin_head_t {
	char magic[16];
	int checksum;
	int datasize;
	/*data*/
};

int read_tcon_bin_data_by_id(char *data_buf, int buf_size, int bin_id)
{
	int ret;
	int i;
	struct tcon_bin_head_t tcon_bin_head;
	int checksum  = 0;
	char *temp_buf = NULL;
	int headLen = 0;
	char *magic[16]={0};
	uint64_t offset;

	if(NULL == data_buf || (bin_id < TCON_B0_SPI || bin_id >= TCON_BIN_MAX)) {
		printf("[%s, %d] bad param, data_buf=0x%x, bin_id=%d\n", __FUNCTION__, __LINE__, data_buf, bin_id);
		return -1;
	}

	if(bin_id == TCON_B0_SPI) {
		strcpy(magic, "tcon_b0_spi");
		offset = CRI_DATA_TCON_B0_SPI_OFFSET;
	} else if (bin_id == TCON_DEMURA_SET) {
		strcpy(magic, "tcon_demura_set");
		offset = CRI_DATA_TCON_DEMURA_SET_OFFSET;
	} else if (bin_id == TCON_DEMURA_LUT) {
		strcpy(magic, "tcon_demura_lut");
		offset = CRI_DATA_TCON_DEMURA_LUT_OFFSET;
	} else if (bin_id == TCON_DEMURA_CRC) {
		strcpy(magic, "tcon_demura_crc");
		offset = CRI_DATA_TCON_DEMURA_CRC_OFFSET;
	} else {
		printf("[%s, %d] bad param, bin_id = %d\n", __FUNCTION__, __LINE__, bin_id);
		return -1;
	}

	memset(&tcon_bin_head, 0x00, sizeof(tcon_bin_head));
	ret = store_read_ops("cri_data", &tcon_bin_head, offset, (uint64_t)sizeof(tcon_bin_head));
	if(0 != ret){
		printf("[%s, %d] read tcon bin data failed, bin_id=%d\n", __FUNCTION__, __LINE__, bin_id);
		return -1;
	}

	if(strcmp(tcon_bin_head.magic, magic))  {
		printf("[%s, %d] no tcon bin data, bin_id=%d\n", __FUNCTION__, __LINE__, bin_id);
		return -1;
	}

	printf("[%s, %d] magic=%s, checksum=0x%x, datasize=%d, bin_id=%d\n", __FUNCTION__, __LINE__,
					tcon_bin_head.magic, tcon_bin_head.checksum, tcon_bin_head.datasize, bin_id);

	if(buf_size != tcon_bin_head.datasize) {
		printf("[%s, %d] bad param, bin_id =%d size:[%d, %d] \n", __FUNCTION__, __LINE__,
													bin_id, buf_size, tcon_bin_head.datasize);
		return -1;
	}
	/*printf("[%s] sizeof(tcon_bo_spi)=%d, [%lld, %d][%lld, %d]\n", __FUNCTION__,
									sizeof(tcon_bin_head),
									(uint64_t)(CRI_DATA_TCON_B0_SPI_OFFSET + sizeof(tcon_bo_spi)),
									(uint64_t)(CRI_DATA_TCON_B0_SPI_OFFSET + sizeof(tcon_bo_spi)),
									(uint64_t)tcon_bin_head.datasize, (uint64_t)tcon_bin_head.datasize);*/
	temp_buf = (char *)malloc(sizeof(tcon_bin_head) + tcon_bin_head.datasize);
	if(NULL == temp_buf) {
		printf("[%s, %d] malloc faild,size=%d\n", __FUNCTION__, __LINE__, sizeof(tcon_bin_head) + tcon_bin_head.datasize);
		return -1;
	}
	memset(temp_buf, 0x00, (sizeof(tcon_bin_head) + tcon_bin_head.datasize));
	ret = store_read_ops("cri_data", temp_buf, offset, (uint64_t)(sizeof(tcon_bin_head) + tcon_bin_head.datasize));
	if(0 != ret){
		printf("[%s, %d] read tcon data failed, bin_id=%d\n", __FUNCTION__, __LINE__, bin_id);
		return -1;
	}

	headLen = sizeof(tcon_bin_head);
	for(i = 0; i < tcon_bin_head.datasize; i ++){
		checksum  += temp_buf[headLen + i];
		//printf("[%s, %d] data_buf[%d] = 0x%x\n", __FUNCTION__, __LINE__, i, temp_buf[headLen + i]);
	}

	checksum  = checksum  & 0xffffffff;

	if (checksum != tcon_bin_head.checksum) {
		printf("[%s, %d] data checksum failed, bin_id=%d\n", __FUNCTION__, __LINE__, bin_id);
		return -1;
	}

	for(i = 0; i < tcon_bin_head.datasize; i ++){
		data_buf[i]  = temp_buf[headLen + i];
	}

	free(temp_buf);
	temp_buf = NULL;

	return 0;
}

int get_tcon_bin_size_by_id(int bin_id)      //return data size, size=0 means not exist
{
	struct tcon_bin_head_t tcon_bin_head;
	int ret;
	char magic[16] = {0};
	uint64_t offset;

	if(bin_id < TCON_B0_SPI || bin_id >= TCON_BIN_MAX) {
		printf("[%s, %d] bad param, bin_id = %d\n", __FUNCTION__, __LINE__, bin_id);
		return -1;
    }

	if(bin_id == TCON_B0_SPI) {
		strcpy(magic, "tcon_b0_spi");
		offset = CRI_DATA_TCON_B0_SPI_OFFSET;
	} else if (bin_id == TCON_DEMURA_SET) {
		strcpy(magic, "tcon_demura_set");
		offset = CRI_DATA_TCON_DEMURA_SET_OFFSET;
	} else if (bin_id == TCON_DEMURA_LUT) {
		strcpy(magic, "tcon_demura_lut");
		offset = CRI_DATA_TCON_DEMURA_LUT_OFFSET;
	} else if (bin_id == TCON_DEMURA_CRC) {
		strcpy(magic, "tcon_demura_crc");
		offset = CRI_DATA_TCON_DEMURA_CRC_OFFSET;
	} else {
		printf("[%s, %d] bad param, bin_id = %d\n", __FUNCTION__, __LINE__, bin_id);
		return -1;
	}

	memset(&tcon_bin_head, 0x00, sizeof(tcon_bin_head));
	ret = store_read_ops("cri_data", &tcon_bin_head, offset, (uint64_t)sizeof(tcon_bin_head));
	if(0 != ret){
		printf("[%s, %d] read tcon bin data failed, bin_id=%d\n", __FUNCTION__, __LINE__, bin_id);
		return 0;
	}

	if(strcmp(tcon_bin_head.magic, magic)) {
		printf("[%s, %d] no tcon bin data, bin_id=%d\n", __FUNCTION__, __LINE__, bin_id);
		return 0;
	}

	printf("[%s, %d] magic=%s, checksum=0x%x, datasize=%d, bin_id=%d\n", __FUNCTION__, __LINE__,
					tcon_bin_head.magic, tcon_bin_head.checksum, tcon_bin_head.datasize, bin_id);

	return tcon_bin_head.datasize;
}

int serial_set_pin_port(unsigned long port_base)
{
    //UART in "Always On Module"
    //GPIOAO_0==tx,GPIOAO_1==rx
    //setbits_le32(P_AO_RTI_PIN_MUX_REG,3<<11);
    return 0;
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

/* secondary_boot_func
 * this function should be write with asm, here, is is only for compiling pass
 * */
void secondary_boot_func(void)
{
}
#ifdef  ETHERNET_INTERNAL_PHY
void internalPhyConfig(struct phy_device *phydev)
{
}

static int dwmac_meson_cfg_pll(void)
{
	writel(0x39C0040A, P_ETH_PLL_CTL0);
	writel(0x927E0000, P_ETH_PLL_CTL1);
	writel(0xAC5F49E5, P_ETH_PLL_CTL2);
	writel(0x00000000, P_ETH_PLL_CTL3);
	udelay(200);
	writel(0x19C0040A, P_ETH_PLL_CTL0);
	return 0;
}

static int dwmac_meson_cfg_analog(void)
{
	/*Analog*/
	writel(0x20200000, P_ETH_PLL_CTL5);
	writel(0x0000c002, P_ETH_PLL_CTL6);
	writel(0x00000023, P_ETH_PLL_CTL7);

	return 0;
}

static int dwmac_meson_cfg_ctrl(void)
{
	/*config phyid should between  a 0~0xffffffff*/
	/*please don't use 44000181, this has been used by internal phy*/
	writel(0x33000180, P_ETH_PHY_CNTL0);

	/*use_phy_smi | use_phy_ip | co_clkin from eth_phy_top*/
	writel(0x260, P_ETH_PHY_CNTL2);

	writel(0x74043, P_ETH_PHY_CNTL1);
	writel(0x34043, P_ETH_PHY_CNTL1);
	writel(0x74043, P_ETH_PHY_CNTL1);
	return 0;
}

static void setup_net_chip(void)
{
	eth_aml_reg0_t eth_reg0;

	eth_reg0.d32 = 0;
	eth_reg0.b.phy_intf_sel = 4;
	eth_reg0.b.rx_clk_rmii_invert = 0;
	eth_reg0.b.rgmii_tx_clk_src = 0;
	eth_reg0.b.rgmii_tx_clk_phase = 0;
	eth_reg0.b.rgmii_tx_clk_ratio = 4;
	eth_reg0.b.phy_ref_clk_enable = 1;
	eth_reg0.b.clk_rmii_i_invert = 1;
	eth_reg0.b.clk_en = 1;
	eth_reg0.b.adj_enable = 1;
	eth_reg0.b.adj_setup = 0;
	eth_reg0.b.adj_delay = 9;
	eth_reg0.b.adj_skew = 0;
	eth_reg0.b.cali_start = 0;
	eth_reg0.b.cali_rise = 0;
	eth_reg0.b.cali_sel = 0;
	eth_reg0.b.rgmii_rx_reuse = 0;
	eth_reg0.b.eth_urgent = 0;
	setbits_le32(P_PREG_ETH_REG0, eth_reg0.d32);// rmii mode

	dwmac_meson_cfg_pll();
	dwmac_meson_cfg_analog();
	dwmac_meson_cfg_ctrl();

	/* eth core clock */
	setbits_le32(HHI_GCLK_MPEG1, (0x1 << 3));
	/* eth phy clock */
	setbits_le32(HHI_GCLK_MPEG0, (0x1 << 4));

	/* eth phy pll, clk50m */
	setbits_le32(HHI_FIX_PLL_CNTL3, (0x1 << 5));

	/* power on memory */
	clrbits_le32(HHI_MEM_PD_REG0, (1 << 3) | (1<<2));
}
#endif

#ifdef ETHERNET_EXTERNAL_PHY

static int dwmac_meson_cfg_drive_strength(void)
{
	writel(0xaaaaaaa5, P_PAD_DS_REG4A);
	return 0;
}

static void setup_net_chip_ext(void)
{
	eth_aml_reg0_t eth_reg0;
	writel(0x11111111, P_PERIPHS_PIN_MUX_6);
	writel(0x111111, P_PERIPHS_PIN_MUX_7);

	eth_reg0.d32 = 0;
	eth_reg0.b.phy_intf_sel = 1;
	eth_reg0.b.rx_clk_rmii_invert = 0;
	eth_reg0.b.rgmii_tx_clk_src = 0;
	eth_reg0.b.rgmii_tx_clk_phase = 1;
	eth_reg0.b.rgmii_tx_clk_ratio = 4;
	eth_reg0.b.phy_ref_clk_enable = 1;
	eth_reg0.b.clk_rmii_i_invert = 0;
	eth_reg0.b.clk_en = 1;
	eth_reg0.b.adj_enable = 0;
	eth_reg0.b.adj_setup = 0;
	eth_reg0.b.adj_delay = 0;
	eth_reg0.b.adj_skew = 0;
	eth_reg0.b.cali_start = 0;
	eth_reg0.b.cali_rise = 0;
	eth_reg0.b.cali_sel = 0;
	eth_reg0.b.rgmii_rx_reuse = 0;
	eth_reg0.b.eth_urgent = 0;
	setbits_le32(P_PREG_ETH_REG0, eth_reg0.d32);// rmii mode

	setbits_le32(HHI_GCLK_MPEG1, 0x1 << 3);
	/* power on memory */
	clrbits_le32(HHI_MEM_PD_REG0, (1 << 3) | (1<<2));
}
#endif

int eth_board_id = 0;   // 0  external trim    1 internal trim
extern struct eth_board_socket* eth_board_setup(char *name);
extern int designware_initialize(ulong base_addr, u32 interface);
extern struct phy_device * p_phydev;
unsigned int cts_setting[16] = {0xA7E00000, 0x87E00000, 0x8BE00000, 0x93E00000,
				0x8FE00000, 0x97E00000,	0x9BE00000, 0xA7E00000,
				0xABE00000, 0xB3E00000, 0xAFE00000, 0xB7E00000,
				0xE7E00000, 0xEFE00000, 0xFBE00000, 0xFFE00000};
int board_eth_init(bd_t *bis)
{
	unsigned int tx_amp_bl2 = 0;
	unsigned int cts_valid = 0;
	unsigned int cts_amp = 0;
#ifdef CONFIG_ETHERNET_NONE
	return 0;
#endif

#ifdef ETHERNET_EXTERNAL_PHY
	dwmac_meson_cfg_drive_strength();
	setup_net_chip_ext();
#endif
#ifdef ETHERNET_INTERNAL_PHY
	setup_net_chip();
#endif
	udelay(1000);
	designware_initialize(ETH_BASE, PHY_INTERFACE_MODE_RMII);
/*tx_amp*/
#ifdef ETHERNET_INTERNAL_PHY
//	tx_amp_bl2 = readl(0xff800270);
	tx_amp_bl2 = readl(AO_SEC_GP_CFG12);
	printf("wzh AO_SEC_GP_CFG12 0x%x\n", readl(AO_SEC_GP_CFG12));

	if (eth_board_id == 0) {
		cts_valid =  (tx_amp_bl2 >> 5) & 0x1;

		if (cts_valid)
			cts_amp  = tx_amp_bl2 & 0xf;
		else
			cts_amp = 1;
	} else if (eth_board_id == 1) {
		cts_valid =  (tx_amp_bl2 >> 4) & 0x1;

		if (cts_valid)
			cts_amp  = tx_amp_bl2 & 0xf;
		else
			cts_amp = 0;
		/*invalid will set cts_setting[0] 0xA7E00000*/
		writel(cts_setting[cts_amp], P_ETH_PLL_CTL3);

		cts_amp = 5;
	}

	phy_write(p_phydev, MDIO_DEVAD_NONE, 0x14, 0x0000);
	phy_write(p_phydev, MDIO_DEVAD_NONE, 0x14, 0x0400);
	phy_write(p_phydev, MDIO_DEVAD_NONE, 0x14, 0x0000);
	phy_write(p_phydev, MDIO_DEVAD_NONE, 0x14, 0x0400);
	phy_write(p_phydev, MDIO_DEVAD_NONE, 0x17, cts_amp);
	phy_write(p_phydev, MDIO_DEVAD_NONE, 0x14, 0x4418);
#endif
	return 0;
}

#if CONFIG_AML_SD_EMMC
#include <mmc.h>
#include <asm/arch/sd_emmc.h>
static int  sd_emmc_init(unsigned port)
{
    switch (port)
	{
		case SDIO_PORT_A:
			break;
		case SDIO_PORT_B:
			/*treat port b as port c on t5*/
			break;
			clrbits_le32(P_PERIPHS_PIN_MUX_9, 0xF << 24);
			setbits_le32(P_PREG_PAD_GPIO1_EN_N, 1 << 6);
			setbits_le32(P_PAD_PULL_UP_EN_REG1, 1 << 6);
			setbits_le32(P_PAD_PULL_UP_REG1, 1 << 6);
			break;
		case SDIO_PORT_C:
			//enable pull up
			//clrbits_le32(P_PAD_PULL_UP_REG3, 0xff<<0);
			break;
		default:
			break;
	}

	return cpu_sd_emmc_init(port);
}

extern unsigned sd_debug_board_1bit_flag;


static void sd_emmc_pwr_prepare(unsigned port)
{
	cpu_sd_emmc_pwr_prepare(port);
}

static void sd_emmc_pwr_on(unsigned port)
{
    switch (port)
	{
		case SDIO_PORT_A:
			break;
		case SDIO_PORT_B:
//            clrbits_le32(P_PREG_PAD_GPIO5_O,(1<<31)); //CARD_8
//            clrbits_le32(P_PREG_PAD_GPIO5_EN_N,(1<<31));
			/// @todo NOT FINISH
			break;
		case SDIO_PORT_C:
			break;
		default:
			break;
	}
	return;
}
static void sd_emmc_pwr_off(unsigned port)
{
	/// @todo NOT FINISH
    switch (port)
	{
		case SDIO_PORT_A:
			break;
		case SDIO_PORT_B:
//            setbits_le32(P_PREG_PAD_GPIO5_O,(1<<31)); //CARD_8
//            clrbits_le32(P_PREG_PAD_GPIO5_EN_N,(1<<31));
			break;
		case SDIO_PORT_C:
			break;
				default:
			break;
	}
	return;
}

// #define CONFIG_TSD      1
static void board_mmc_register(unsigned port)
{
	struct aml_card_sd_info *aml_priv=cpu_sd_emmc_get(port);
    if (aml_priv == NULL)
		return;

	aml_priv->sd_emmc_init=sd_emmc_init;
	aml_priv->sd_emmc_detect=sd_emmc_detect;
	aml_priv->sd_emmc_pwr_off=sd_emmc_pwr_off;
	aml_priv->sd_emmc_pwr_on=sd_emmc_pwr_on;
	aml_priv->sd_emmc_pwr_prepare=sd_emmc_pwr_prepare;
	aml_priv->desc_buf = malloc(NEWSD_MAX_DESC_MUN*(sizeof(struct sd_emmc_desc_info)));

	if (NULL == aml_priv->desc_buf)
		printf(" desc_buf Dma alloc Fail!\n");
	else
		printf("aml_priv->desc_buf = 0x%p\n",aml_priv->desc_buf);

	sd_emmc_register(aml_priv);
}
int board_mmc_init(bd_t	*bis)
{
#ifdef CONFIG_VLSI_EMULATOR
	//board_mmc_register(SDIO_PORT_A);
#else
	//board_mmc_register(SDIO_PORT_B);
#endif
	board_mmc_register(SDIO_PORT_B);
	board_mmc_register(SDIO_PORT_C);
//	board_mmc_register(SDIO_PORT_B1);
	return 0;
}
#endif

#if defined(CONFIG_BOARD_EARLY_INIT_F)
int board_early_init_f(void){
	/*add board early init function here*/
	return 0;
}
#endif

#ifdef CONFIG_USB_XHCI_CRG_AMLOGIC
#include <asm/arch/usb-v2.h>
#include <asm/arch/gpio.h>
#define CONFIG_GXL_USB_U2_PORT_NUM	CONFIG_USB_U2_PORT_NUM

#define CONFIG_GXL_USB_U3_PORT_NUM	0

static void gpio_set_vbus_power(char is_power_on)
{
	int ret;

	ret = gpio_request(CONFIG_USB_GPIO_PWR,
		CONFIG_USB_GPIO_PWR_NAME);
	if (ret && ret != -EBUSY) {
		printf("gpio: requesting pin %u failed\n",
			CONFIG_USB_GPIO_PWR);
		return;
	}

	printf("gpio_set_vbus_power=%d\n", is_power_on);

	if (is_power_on) {
		gpio_direction_output(CONFIG_USB_GPIO_PWR, 1);
	} else {
		gpio_direction_output(CONFIG_USB_GPIO_PWR, 0);
	}
}

struct amlogic_usb_config g_usb_config_GXL_skt={
	CONFIG_GXL_XHCI_BASE,
	USB_ID_MODE_HARDWARE,
	gpio_set_vbus_power, //set_vbus_power
	CONFIG_GXL_USB_PHY2_BASE,
	CONFIG_GXL_USB_PHY3_BASE,
	CONFIG_GXL_USB_U2_PORT_NUM,
	CONFIG_GXL_USB_U3_PORT_NUM,
	.usb_phy2_pll_base_addr = {
		CONFIG_USB_PHY_20,
		CONFIG_USB_PHY_21,
		CONFIG_USB_PHY_22,
	}
};
#endif /*CONFIG_USB_XHCI_AMLOGIC*/

#ifdef CONFIG_AML_HDMITX20
static void hdmi_tx_set_hdmi_5v(void)
{
}
#endif

/*
 * mtd nand partition table, only care the size!
 * offset will be calculated by nand driver.
 */
#ifdef CONFIG_AML_MTD
static struct mtd_partition normal_partition_info[] = {
#ifdef CONFIG_DISCRETE_BOOTLOADER
    /* MUST NOT CHANGE this part unless u know what you are doing!
     * inherent parition for descrete bootloader to store fip
     * size is determind by TPL_SIZE_PER_COPY*TPL_COPY_NUM
     * name must be same with TPL_PART_NAME
     */
    {
        .name = "tpl",
        .offset = 0,
        .size = 0,
    },
#endif
    {
        .name = "logo",
        .offset = 0,
        .size = 2*SZ_1M,
    },
    {
        .name = "recovery",
        .offset = 0,
        .size = 16*SZ_1M,
    },
    {
        .name = "boot",
        .offset = 0,
        .size = 16*SZ_1M,
    },
    {
        .name = "system",
        .offset = 0,
        .size = 256*SZ_1M,
    },
	/* last partition get the rest capacity */
    {
        .name = "data",
        .offset = MTDPART_OFS_APPEND,
        .size = MTDPART_SIZ_FULL,
    },
};
struct mtd_partition *get_aml_mtd_partition(void)
{
	return normal_partition_info;
}
int get_aml_partition_count(void)
{
	return ARRAY_SIZE(normal_partition_info);
}
#endif /* CONFIG_AML_MTD */

#ifdef CONFIG_AML_SPIFC
#include <asm/arch/gpio.h>
#define SPIFC_NUM_CS 1
static int spifc_cs_gpios[SPIFC_NUM_CS] = {GPIOEE(GPIOB_13)};

static int spifc_pinctrl_enable(void *pinctrl, bool enable)
{
	unsigned int val;

	/* mux gpiob_3,4,5,6,7 to spifc */
	val = readl(P_PERIPHS_PIN_MUX_0);
	val &= ~(0xfffff << 12);
	if (enable)
		val |= 0x33333 << 12;
	writel(val, P_PERIPHS_PIN_MUX_0);

	/* mux gpiob_13 to gpio */
	val = readl(P_PERIPHS_PIN_MUX_1);
	val &= ~(0xf << 20);
	writel(val, P_PERIPHS_PIN_MUX_1);

	/* set ds to 3 */
	val = readl(P_PAD_DS_REG0A);
	val |= ((0x3ff << 6) | (0x3 << 26));
	writel(val, P_PAD_DS_REG0A);
	return 0;
}

static const struct spifc_platdata spifc_platdata = {
	.reg = 0xffd14000,
	.mem_map = 0xf6000000,
	.pinctrl_enable = spifc_pinctrl_enable,
	.num_chipselect = SPIFC_NUM_CS,
	.cs_gpios = spifc_cs_gpios,
};

U_BOOT_DEVICE(spifc) = {
	.name = "spifc",
	.platdata = &spifc_platdata,
};
#endif /* CONFIG_AML_SPIFC */

#ifdef CONFIG_AML_SPICC
/* generic config in arch gpio/clock.c */
extern int spicc0_clk_set_rate(int rate);
extern int spicc0_clk_enable(bool enable);
extern int spicc0_pinctrl_enable(bool enable);

static const struct spicc_platdata spicc0_platdata = {
	.compatible = "amlogic,meson-g12a-spicc",
	.reg = (void __iomem *)0xffd13000,
	.clk_rate = 666666666,
	.clk_set_rate = spicc0_clk_set_rate,
	.clk_enable = spicc0_clk_enable,
	.pinctrl_enable = spicc0_pinctrl_enable,
	/* case one slave without cs: {"no_cs", 0} */
	.cs_gpio_names = {"GPIOH_8", 0},
};

U_BOOT_DEVICE(spicc0) = {
	.name = "spicc",
	.platdata = &spicc0_platdata,
};
#endif /* CONFIG_AML_SPICC */

extern void aml_pwm_cal_init(int mode);

#ifdef CONFIG_SYS_I2C_MESON
static const struct meson_i2c_platdata i2c_data[] = {
	{ 0, 0xffd1f000, 166666666, 3, 15, 100000 },
	{ 1, 0xffd1e000, 166666666, 3, 15, 100000 },
	{ 2, 0xffd1d000, 166666666, 3, 15, 100000 },
};

U_BOOT_DEVICES(meson_i2cs) = {
	{ "i2c_meson", &i2c_data[0] },
	{ "i2c_meson", &i2c_data[1] },
	{ "i2c_meson", &i2c_data[2] },
};

/*
 *GPIOH_20//I2C_SCL
 *GPIOH_21//I2C_SDA
 *pinmux configuration seperated with i2c controller configuration
 * config it when you use
 */
#if 1 /* i2c pinmux demo */
void set_i2c_b_pinmux(void)
{
	/*ds =3 */
	setbits_le32(PAD_DS_REG2B, 0xf << 8);
	/*pull up disable*/
	clrbits_le32(PAD_PULL_UP_EN_REG2, 0x3 << 20);
	/*pin mux to i2cm1*/
	clrbits_le32(PERIPHS_PIN_MUX_7, 0xff << 16);
	setbits_le32(PERIPHS_PIN_MUX_7, 0x1 << 16 | 0x1 << 20);
	return;
}
#endif
#endif /*end CONFIG_SYS_I2C_MESON*/

#ifdef CONFIG_PWM_MESON
static const struct meson_pwm_platdata pwm_data[] = {
	{ PWM_AB, 0xffd1b000, IS_DOUBLE_CHANNEL, IS_BLINK },
	{ PWM_CD, 0xffd1a000, IS_DOUBLE_CHANNEL, IS_BLINK },
	{ PWM_EF, 0xffd19000, IS_DOUBLE_CHANNEL, IS_BLINK },
};

U_BOOT_DEVICES(meson_pwm) = {
	{ "amlogic,general-pwm", &pwm_data[0] },
	{ "amlogic,general-pwm", &pwm_data[1] },
	{ "amlogic,general-pwm", &pwm_data[2] },
};
#endif /*end CONFIG_PWM_MESON*/

int board_init(void)
{
	//keep usb tool at first place of board_init
#ifdef CONFIG_AML_V3_FACTORY_BURN
	if ((0x1b8ec003 != readl(P_PREG_STICKY_REG2)) && (0x1b8ec004 != readl(P_PREG_STICKY_REG2)))
	{ aml_v3_factory_usb_burning(0, gd->bd); }
#endif// #ifdef CONFIG_AML_V3_FACTORY_BURN

#ifdef CONFIG_USB_XHCI_CRG_AMLOGIC
	board_usb_pll_disable(&g_usb_config_GXL_skt);
	board_usb_init(&g_usb_config_GXL_skt,BOARD_USB_MODE_HOST);
#endif /*CONFIG_USB_XHCI_AMLOGIC*/
#ifdef CONFIG_SYS_I2C_MESON
	set_i2c_b_pinmux();
#endif
#ifdef UFBL_FEATURE_TEMP_UNLOCK
	amzn_save_temp_unlock_data();
#endif
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
/* Reset BT-module */
void reset_mt7668(void)
{
#if 0
	/* Reset BT-module by reset-pin -- GPIOAO_5 */
	clrbits_le32(P_AO_GPIO_O_EN_N, 1 << 5);
	clrbits_le32(P_AO_GPIO_O_EN_N, 1 << 21);
	mdelay(200);
	setbits_le32(P_AO_GPIO_O_EN_N, 1 << 21);
	mdelay(100);
#else
	/* Reset BT-module by reset-pin -- GPIOD_2 */
	run_command("gpio clear GPIOD_2",0);
	mdelay(200);
	run_command("gpio set GPIOD_2",0);
	mdelay(100);
#endif
}

void set_dts_status(void)
{
	if(run_command("query DTS", 0) == 1){
		setenv("DTS_enabled","yes");
	} else {
		setenv("DTS_enabled","no");
	}
}

void set_dolby_status(void)
{
	if(run_command("query Dolby", 0) == 1){
		setenv("Dolby_enabled","yes");
	} else {
		setenv("Dolby_enabled","no");
	}
}

int board_late_init(void)
{
	TE(__func__);

	char outputModePre[30];
	char outputModeCur[30];
	char *rebootmode = NULL;
	char model_name[256] = "";
	strcpy(outputModePre,getenv("outputmode"));

	amazon_hardware_init();

	run_command("gpio set GPIOH_14",0);
	run_command("gpio set GPIOH_19",0);

	int bl_status = get_lcd_bl_status();
	setenv("bl_status", bl_status ? "1":"0");

	//update env before anyone using it
	run_command("get_rebootmode; echo reboot_mode=${reboot_mode}; "\
					"if test ${reboot_mode} = factory_reset; then "\
						"defenv_reserv;setenv upgrade_step 2;save;"\
					"else if test ${reboot_mode} = quiescent; then "\
						"setenv lcd_init_level 1;"\
					"else if test ${reboot_mode} = recovery_quiescent; then "\
						"setenv lcd_init_level 1;fi;fi;fi;", 0);
	if (bl_status == 0 )
	{
		run_command("if test ${reboot_mode} = watchdog_reboot; then "\
						"setenv lcd_init_level 1;"\
					 "else if test ${reboot_mode} = kernel_panic; then "\
						"setenv lcd_init_level 1;"\
					"else if test ${reboot_mode} = crash_dump; then "\
						"setenv lcd_init_level 1;fi;fi;fi;", 0);
	}

	run_command("if itest ${upgrade_step} == 1; then "\
					"defenv_reserv; setenv upgrade_step 2; saveenv; fi;", 0);
	/*set lcd pmu power if lcd off power up*/
	run_command("get_rebootmode", 0);
	rebootmode = getenv("reboot_mode");
	if(!strcmp(rebootmode,"quiescent") ||
	   !strcmp(rebootmode,"recovery_quiescent") ||
	   ((!strcmp(rebootmode,"watchdog_reboot") ||
	     !strcmp(rebootmode,"kernel_panic") ||
	     !strcmp(rebootmode,"crash_dump")) && bl_status == 0))
	{
		if (idme_get_var_external("model_name", model_name, sizeof(model_name)) != 0) {
			printf("can not get model_name ! \n");
        	}
		printf("get idme model_name: %s\n", model_name);
		if(strstr(model_name, "HVT_FHD_32_1_T") != NULL ||
		(strstr(model_name, "HVT_FHD_32_4_T") != NULL) ||
		(strstr(model_name, "HVT_FHD_32_8_T") != NULL) ||
		(strstr(model_name, "HVT_FHD_43_3_T") != NULL) ||
		(strstr(model_name, "HVT_FHD_43_6_T") != NULL)){
			run_command("gpio set GPIOH_7",0);
		}
	}
	/*add board late init function here*/
#ifndef DTB_BIND_KERNEL
		int ret;
		ret = run_command("store dtb read $dtb_mem_addr", 1);
        if (ret) {
				printf("%s(): [store dtb read $dtb_mem_addr] fail\n", __func__);
#ifdef CONFIG_DTB_MEM_ADDR
				char cmd[64];
				printf("load dtb to %x\n", CONFIG_DTB_MEM_ADDR);
				sprintf(cmd, "store dtb read %x", CONFIG_DTB_MEM_ADDR);
				ret = run_command(cmd, 1);
                if (ret) {
						printf("%s(): %s fail\n", __func__, cmd);
				}
#endif
		}
#elif defined(CONFIG_DTB_MEM_ADDR)
		{
				char cmd[128];
				int ret;
                if (!getenv("dtb_mem_addr")) {
						sprintf(cmd, "setenv dtb_mem_addr 0x%x", CONFIG_DTB_MEM_ADDR);
						run_command(cmd, 0);
				}
				sprintf(cmd, "imgread dtb boot ${dtb_mem_addr}");
				ret = run_command(cmd, 0);
                if (ret) {
						printf("%s(): cmd[%s] fail, ret=%d\n", __func__, cmd, ret);
				}
		}
#endif// #ifndef DTB_BIND_KERNEL
		run_command("setenv logo_name bootup", 1);

		amazon_config_init();
		/* load unifykey */
		run_command("keyunify init 0x1234", 0);
		set_dolby_status();
		set_dts_status();
		get_build_tag();

#ifdef CONFIG_AML_VPU
	vpu_probe();
#endif
#ifdef CONFIG_AML_VPP
	vpp_init();
#endif

	run_command("ini_model", 0);
#ifdef CONFIG_AML_HDMITX20
	hdmi_tx_set_hdmi_5v();
	hdmi_tx_init();
#endif
#ifdef CONFIG_AML_CVBS
	run_command("cvbs init", 0);
#endif
#ifdef CONFIG_AML_LCD
	lcd_probe();
#endif

#ifdef CONFIG_AML_V3_FACTORY_BURN
	if (0x1b8ec003 == readl(P_PREG_STICKY_REG2))
		aml_v3_factory_usb_burning(1, gd->bd);
#endif// #ifdef CONFIG_AML_V3_FACTORY_BURN
#ifdef CONFIG_AML_FACTORY_BURN_LOCAL_UPGRADE //try auto upgrade from ext-sdcard
	aml_try_factory_sdcard_burning(0, gd->bd);
#endif//#ifdef CONFIG_AML_FACTORY_BURN_LOCAL_UPGRADE

	TE(__func__);
	strcpy(outputModeCur,getenv("outputmode"));
	if (strcmp(outputModeCur,outputModePre)) {
		printf("uboot outputMode change saveenv old:%s - new:%s\n",outputModePre,outputModeCur);
		run_command("saveenv", 0);
	}
/*IDME have beed initialized, remove it*/
/*
#ifdef CONFIG_IDME
	idme_initialize();
#endif
*/
	printf("amzn-----------------bootloader---------up\n");
	return 0;
}
#endif

#ifdef CONFIG_AML_TINY_USBTOOL
int usb_get_update_result(void)
{
	unsigned long upgrade_step;
	upgrade_step = simple_strtoul (getenv ("upgrade_step"), NULL, 16);
	printf("upgrade_step = %d\n", (int)upgrade_step);
	if (upgrade_step == 1)
	{
		run_command("defenv", 1);
		run_command("setenv upgrade_step 2", 1);
		run_command("saveenv", 1);
		return 0;
	}
	else
	{
		return -1;
	}
}
#endif

phys_size_t get_effective_memsize(void)
{
	// >>16 -> MB, <<20 -> real size, so >>16<<20 = <<4
#if defined(CONFIG_SYS_MEM_TOP_HIDE)
	return (((readl(AO_SEC_GP_CFG0)) & 0xFFFF0000) << 4) - CONFIG_SYS_MEM_TOP_HIDE;
#else
	return (((readl(AO_SEC_GP_CFG0)) & 0xFFFF0000) << 4);
#endif
}

static int get_amp_name(char *amp_name, int size)
{
	int ret = -1;
	const char *ini_value = NULL;

	IniParserInit();

	if (IniParseFile(get_model_sum_path()) < 0) {
		printf("%s, model ini load file error!\n", __func__);
		goto exit;
	}

	ini_value = IniGetString("DEFAULT", "AMP", "null");
	if (strcmp(ini_value, "null") == 0) {
		printf("%s, get AMP item failed!\n", __func__);
		goto exit;
	}

	memset(amp_name, 0 , size);
	strncpy(amp_name, ini_value, size - 1);
	printf("%s, amp name is %s!\n", __func__, amp_name);
	ret = 0;
exit:
	IniParserUninit();
	return ret;
}

#ifdef CONFIG_MULTI_DTB
int checkhw(char * name)
{
    /*if use ddr size to identify*/
	unsigned int ddr_size = 0;
	char dtb_name[64] = {0};
	char product_name[64] = {0};
	char amp_name[64] = "";
	char model_name[256] = "";

	int i;
	for (i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		ddr_size += gd->bd->bi_dram[i].size;
	}
	cpu_id_t  cpu_id = get_cpu_id();

#if defined(CONFIG_SYS_MEM_TOP_HIDE)
	ddr_size += CONFIG_SYS_MEM_TOP_HIDE;
#endif
	switch (ddr_size) {
		case 0x40000000:
			if (cpu_id.chip_rev == 0xA) {
				printf("----OLNY SURPPORT t950d4 1G on REVA----\n");
				if (MESON_CPU_PACKAGE_ID_T950X4 == cpu_id.package_id)
					strcpy(dtb_name, "t5d-reva_t950d4_proto-am301-1g\0");
				else {
					strcpy(dtb_name, "t5d-reva_t950d4_proto-am301-1g\0");
				}
				setenv("cpu_version", "rev_a");
			} else {
				printf("----DON'T SURPPORT t950d4 1G on REVB----\n");
				setenv("cpu_version", "rev_b");
				return;
/*
				if (MESON_CPU_PACKAGE_ID_T950X4 == cpu_id.package_id)
					strcpy(dtb_name, "t5d_t950x4_am311-1g\0");
				else {
					strcpy(dtb_name, "t5d_t950d4_am301-1g\0");
				}
*/
			}

			setenv("mem_size", "1g");
			break;
		case 0x20000000:
			if (cpu_id.chip_rev == 0xA) {
				if (MESON_CPU_PACKAGE_ID_T950X4 == cpu_id.package_id)
					strcpy(dtb_name, "t5d-reva_t950x4_am311-512m\0");
				else
					strcpy(dtb_name, "t5d-reva_t950d4_am301-512m\0");
				setenv("cpu_version", "rev_a");
			} else {
				if (MESON_CPU_PACKAGE_ID_T950X4 == cpu_id.package_id)
					strcpy(dtb_name, "t5d_t950x4_am311-512m\0");
				else
					strcpy(dtb_name, "t5d_t950d4_am301-512m\0");
				setenv("cpu_version", "rev_b");
			}
			setenv("mem_size", "512m");
			break;
		case 0x60000000:
			idme_get_var_external("product_name", product_name, sizeof(product_name));
			if (strcmp("hadrian", product_name) == 0) {
				if ( get_amp_name( amp_name, sizeof(amp_name)) == 0 ) {
					if ( strcmp("tas5805", amp_name ) == 0 )
						strcpy(dtb_name, "t5d_t950d4_hadrian-1.5g\0");
					else
						strcpy(dtb_name, "t5d_t950d4_hadrian-1.5g\0");
				} else
					strcpy(dtb_name, "t5d_t950d4_hadrian-1.5g\0");
			} else
				strcpy(dtb_name, "t5d_t950d4_hadrian-1.5g\0");

			setenv("cpu_version", "rev_b");
			setenv("mem_size", "1.5g");
			break;
		default:
			strcpy(dtb_name, "t5d_am301_unsupport");
			setenv("cpu_version", "rev_a");
			break;
	}
	strcpy(name, dtb_name);
	setenv("aml_dt", dtb_name);
	return 0;
}
#endif

const char * const _env_args_reserve_[] =
{
		"aml_dt",
		"firstboot",
		"lock",
		"upgrade_step",
		"model_name",

		NULL//Keep NULL be last to tell END
};

#ifdef UFBL_FEATURE_ONETIME_UNLOCK
int do_onetimeunlock(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = -1;
    unsigned char *b64_code;
    unsigned char *b64_cert;
    void * new_code;
    void * new_cert;
    unsigned int out_len_code;
    unsigned int out_len_cert;

    if (argc < 2) {
		ret = -1;
		goto done;
    }
    if (!strcmp(argv[1], "getcode")) {
        unsigned char one_tu_code[ONETIME_UNLOCK_CODE_LEN + 1] = {0};
        unsigned int unlock_code_len = sizeof(one_tu_code);

        if (amzn_get_one_tu_code(one_tu_code, &unlock_code_len)) {
	    printf("cannot get onetime unlock code\n");
	    ret = -2;
            goto done;
         } else {
	    printf("%s\n", one_tu_code);
         }
    } else if (!strcmp(argv[1], "setcode")) {

        if(argc < 3) {
	    ret = -3;
            goto done;
        }
        b64_code = (unsigned char *)argv[2];
	out_len_code = strlen(b64_code);
	if((b64_code == NULL) || (out_len_code == 0)){
		printf("can not get code, please re-try !\n");
		ret = -3;
		goto done;
	}
	printf("code lenth %d \n",out_len_code);
	new_code = malloc(out_len_code+1);
	if (new_code == NULL) {
		printf("memory is NULL !\n");
		ret = -3;
		goto done;
	}
	memset(new_code,0,out_len_code+1);
	memcpy(new_code,b64_code,out_len_code);
	if (amzn_set_onetime_unlock_code(new_code, out_len_code)) {
		printf("try again \n");
		if (amzn_set_onetime_unlock_code(new_code, out_len_code)) {
			printf("set onetime unlock code error\n");
			ret = -4;
			goto done;
		}
		printf("set onetime unlock code OKAY\n");
        } else {
		printf("set onetime unlock code OKAY\n");
        }
    } else if (!strcmp(argv[1], "setcert")) {

        if(argc < 3) {
	    ret = -5;
            goto done;
        }
	b64_cert = (unsigned char *)argv[2];
	out_len_cert = strlen(b64_cert);
	if((b64_cert == NULL) || (out_len_cert == 0)){
		printf("can not get cert, please re-try !\n");
		ret = -5;
		goto done;
	}
	// do not need to free in uboot
	printf("cert lenth %d \n",out_len_cert);
	new_cert = malloc(out_len_cert+1);
	if (new_cert == NULL) {
		printf("memory is NULL !\n");
		ret = -5;
		goto done;
	}
	memset(new_cert,0,out_len_cert+1);
	memcpy(new_cert,b64_cert,out_len_cert);

        if (amzn_set_onetime_unlock_cert(new_cert, out_len_cert)) {
		printf("try again \n");
		if (amzn_set_onetime_unlock_cert(new_cert, out_len_cert)) {
			printf("set onetime unlock cert error\n");
			ret = -6;
			goto done;
		}
		printf("set onetime unlock cert OKAY\n");
        } else {
		printf("set onetime unlock cert OKAY\n");
        }
    } else {
	ret = -10;
	goto done;
    }
	ret = 0;
done:
    if (ret)
	printf("do_onetimeunlock fail: %d\n", ret);
    else
	printf("do_onetimeunlock pass\n");
    return 0;
}

U_BOOT_CMD(
     onetimeunlock ,    CONFIG_SYS_MAXARGS,    1,     do_onetimeunlock,
     "onetimeunlock   - one time unlock\n",
     "[onetimeunlock getcode]\n"
     "[onetimeunlock setcode signed_code]\n"
     "[onetimeunlock setcert signed_cert]\n"
);

#endif

#ifdef UFBL_FEATURE_TEMP_UNLOCK
int do_tempunlock(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = -1;
	unsigned char *b64_code = NULL;
	unsigned char *b64_cert = NULL;
	void *new_code = NULL;
	void *new_cert = NULL;
	unsigned int out_len_code = 0;
	unsigned int out_len_cert = 0;

	if (argc < 2) {
		ret = -1;
		goto done;
	}

	if (!strcmp(argv[1], "getcode")) {
		unsigned char tu_code[BASE64_LEN(TEMP_UNLOCK_CODE_LEN) + 1] = {0};
		unsigned int tu_code_len = sizeof(tu_code);
		if (amzn_get_temp_unlock_current_code(tu_code, &tu_code_len)) {
			printf("cannot get temp unlock code\n");
			ret = -2;
			goto done;
		} else {
			printf("%s\n", tu_code);
		}
	} else if (!strcmp(argv[1], "setcode")) {
		if (argc < 3) {
			ret = -3;
			goto done;
		}

		b64_code = (unsigned char *)argv[2];
		out_len_code = strlen(b64_code);
		if((b64_code == NULL) || (out_len_code == 0)){
			printf("can not get code, please re-try !\n");
			ret = -4;
			goto done;
		}
		printf("code lenth %d \n", out_len_code);

		new_code = malloc(out_len_code + 1);
		if (new_code == NULL) {
			printf("memory is NULL !\n");
			ret = -5;
			goto done;
		}
		memset(new_code, 0, out_len_code + 1);
		memcpy(new_code, b64_code, out_len_code);

		if (amzn_set_temp_unlock_idme_code(new_code, out_len_code)) {
			printf("set temp unlock code error\n");
			ret = -6;
			goto done;
		}
		printf("set temp unlock code OKAY\n");
	} else if (!strcmp(argv[1], "setcert")) {
		if (argc < 3) {
			ret = -7;
			goto done;
		}

		b64_cert = (unsigned char *)argv[2];
		out_len_cert = strlen(b64_cert);
		if((b64_cert == NULL) || (out_len_cert == 0)){
			printf("can not get cert, please re-try !\n");
			ret = -8;
			goto done;
		}

		printf("cert lenth %d \n",out_len_cert);
		new_cert = malloc(out_len_cert + 1);
		if (new_cert == NULL) {
			printf("memory is NULL !\n");
			ret = -9;
			goto done;
		}
		memset(new_cert, 0, out_len_cert + 1);
		memcpy(new_cert, b64_cert, out_len_cert);

		if (amzn_set_temp_unlock_idme_cert(new_cert, out_len_cert)) {
			printf("set temp unlock cert error\n");
			ret = -10;
			goto done;
		}
		printf("set temp unlock cert OKAY\n");
	} else {
		ret = -10;
		goto done;
	}
	ret = 0;

done:
	free(new_code);
	free(new_cert);
	if (ret)
		printf("do_tempunlock fail: %d\n", ret);
	else
		printf("do_tempunlock pass\n");
	return ret;
}

U_BOOT_CMD(
     tempunlock ,    CONFIG_SYS_MAXARGS,    1,     do_tempunlock,
     "tempunlock   - temp unlock\n",
     "[tempunlock getcode]\n"
     "[tempunlock setcode signed_code]\n"
     "[tempunlock setcert signed_cert]\n"
);

#endif

#ifdef UFBL_FEATURE_UNLOCK
extern int idme_auto_clean_before_relock(void);

int do_relock(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = -1;

    if (idme_update_var_ex("unlock_code", "", 0)
#ifdef UFBL_FEATURE_TEMP_UNLOCK
            || amzn_clear_temp_unlock_idme()
#endif
            ) {
        printf("do_relock failed\n");
    } else {
        ret = 0;
        printf("do_relock pass\n");
    }

    return ret;
}

U_BOOT_CMD(
     relock ,    CONFIG_SYS_MAXARGS,    1,     do_relock,
     "Relock device\n",
     "\n"
);

#endif

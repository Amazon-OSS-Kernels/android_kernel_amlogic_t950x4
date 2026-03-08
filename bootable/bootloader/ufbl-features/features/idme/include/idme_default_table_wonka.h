/*
 * idme_default_table_wonka.h
 *
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

/*!
 * @file idme_default_table_wonka.h
 * @brief This file contains idme default table values
 *  in the userstore partition
 *
 */

#ifndef __IDME_DEFAULT_TABLE_WONKA_H__

/* If size field in idme_desc is updated, need to make sure
   the total size of all idme fields are  within CONFIG_IDME_SIZE
   in include/idme.h. */
const struct idme_init_values idme_default_values[] = {
	{ { "board_id", 16, 1, 0444 },
		/* Default Board ID value */
		"ffffff0000000000"
	},
	{ { "serial", 16, 1, 0444 },
		/* Default DSN value */
		"0"
	},
	{ { "mac_addr", 16, 1, 0444 },
		/* Default MAC address */
		"0"
	},
	{ { "mac_sec", 32, 1, 0440 },
		/* Default MAC secret */
		"0"
	},
	{ { "bt_mac_addr", 16, 1, 0444 },
		/* Default BT MAC address */
		"0"
	},
	{ { "bt_mfg", 128, 1, 0444 },
		/* Default BT MFG value */
		"0"
	},
	{ { "wifi_mfg", 1024, 1, 0444 },
		/* Default WIFI MFG value */
		"0"
	},
	{ { "eth_mac_addr", 16, 1, 0444 },
		/* Default MAC address for ethernet */
		"0"
	},
	{ { "eth_ip_addr", 16, 1, 0444 },
		/* Initial default IPAddress */
		"0"
	},
	{ { "product_name", 32, 1, 0444 },
		/* Product name, acos 2.4 */
		"0"
	},
	{ { "productid", 32, 1, 0444 },
		/* Default Primary Product ID */
		"0"
	},
	{ { "productid2", 32, 1, 0444 },
		/* Default Secondary Product ID */
		"0"
	},
	{ { "bootmode", 4, 1, 0444 },
		/* Default Bootmode */
		"1"
	},
	{ { "postmode", 4, 1, 0444 },
		/* Default Postmode */
		"0"
	},
	{ { "bootcount", 8, 1, 0444 },
		/* Initial Bootcount */
		"0"
	},
	{ { "manufacturing", 512, 1, 0444 },
		/* Manufacturer-specific data */
		""
	},
	{ { "unlock_code", 1024, 1, 0444 },
		/* Unlock code */
		""
	},
	{ { "miccal.0", 16, 1, 0444 },
		/* MIC Calibration Data for ABC */
		"0"
	},
	{ { "miccal.1", 16, 1, 0444 },
		/* MIC Calibration Data */
		"0"
	},
	{ { "miccal.2", 16, 1, 0444 },
		/* MIC Calibration Data */
		"0"
	},
	{ { "miccal.3", 16, 1, 0444 },
		/* MIC Calibration Data */
		"0"
	},
	{ { "miccal.4", 16, 1, 0444 },
		/* MIC Calibration Data */
		"0"
	},
	{ { "miccal.5", 16, 1, 0444 },
		/* MIC Calibration Data */
		"0"
	},
	{ { "miccal.6", 16, 1, 0444 },
		/* MIC Calibration Data */
		"0"
	},
	{ { "miccal.7", 16, 1, 0444 },
		/* MIC Calibration Data */
		"0"
	},
	{ { "alscal", 1024, 1, 0444 },
		/* Als Calibration Data for ABC */
		""
	},
	{ { "device_type_id", 32, 1, 0444 },
		/* Initial device type id */
		"0"
	},
	{ { "dev_flags", 8, 1, 0444 },
		/* device specific flag */
		"0"
	},
	{ { "fos_flags", 8, 1, 0444 },
		/* device specific flag */
		"0"
	},
	{ { "usr_flags", 8, 1, 0444 },
		/* device specific flag */
		"0"
	},
	{ { "mfg.locale", 64, 1, 0444},
		/* mfg.locale, current values are en-US, de-DE, en-GB */
		"en-US"
	},
	{ { "mfr_name", 32, 1, 0444 },
		/* Manufacturer name */
		"0"
	},
	{ { "mfr_model", 32, 1, 0444 },
		/* Manufacturer model */
		"0"
	},
        { { "product_model", 32, 1, 0444 },
                /* product model */
                "0"
        },
	{ { "transition_done", 4, 1, 0444 },
		/* transition done flag */
		"0"
	},
	{ { "country_code", 8, 1, 0444 },
		/* country_code */
		"0"
	},
	{ { "assy_sn", 32, 1, 0444 },
		/* assy_sn */
		"0"
	},
	{ { "res1", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res2", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res3", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res4", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res5", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res6", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res7", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res8", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "keys", 122888, 1, 0444 },
		/* for backup of keys, 120k */
		""
	},
	{ { "ledparams", 64, 1, 0444 },
		/* Additional LED Parameters */
		"ledcalibparams=1,0x007f7f7f,0x00ffffff"
	},
	{ { "hw_variant", 32, 1, 0444 },
		/* Wonka Hardware Variant, default is smitap*/
		"smitap"
	},
	{ { "", 0, 0, 0 }, 0 },
};


#endif /* __IDME_DEFAULT_TABLE_WONKA_H__ */

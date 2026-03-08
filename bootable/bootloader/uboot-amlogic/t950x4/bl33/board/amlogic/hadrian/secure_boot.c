/*
 * secure_boot.c
 *
 * Copyright 2020 - 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

#include <asm-generic/gpio.h>
#include <asm/arch/secure_apb.h>
#include <asm/io.h>
#include <common.h>
#include <ctype.h>
#include "amzn_secure_boot.h"
#if defined(UFBL_FEATURE_UNLOCK)
#include <amzn_unlock.h>
#include <u-boot/sha256.h>
#endif
#if defined(UFBL_FEATURE_ONETIME_UNLOCK)
#include "onetime_unlock_key.h"
#include <amzn_onetime_unlock.h>
#endif
#if defined(UFBL_FEATURE_TEMP_UNLOCK)
#include <config.h>
#include <asm/arch/io.h>
#include <malloc.h>
#include <asm/arch/secure_apb.h>
#include <tee.h>
#include <errno.h>
#include <amzn_temp_unlock.h>
#include "amzn_temp_unlock_aml_impl.h"
// temp unlock share the same key with onetime unlock
#include "onetime_unlock_key.h"
#endif

/**
 * Minimum Version Number register
 *
 * MVN1 holds (BL30 | BL31 | BL32 | BL33)
 * MVN2 holds (BL2 | FIP_HEADER | reserved[0..15])
 */
#define ANTIROLLBACK_MVN_REG1	(AO_SEC_SD_CFG11)
#define ANTIROLLBACK_MVN_REG2	(AO_SEC_SD_CFG13)

bool secure_boot_enabled(void)
{
	const unsigned long cfg10 = readl(AO_SEC_SD_CFG10);
	return ( (cfg10 & (0x1<< 4)) ? true : false );
	/* 4th bit indicates secure boot status */
}

void read_arb_version(uint32_t *mvn_1_p, uint32_t *mvn_2_p)
{
	uint32_t mvn_1 = 0;
	uint32_t mvn_2 = 0;
	*mvn_1_p = mvn_1 = readl(ANTIROLLBACK_MVN_REG1);
	*mvn_2_p = mvn_2 = readl(ANTIROLLBACK_MVN_REG2);
	printf("mvn_1=0X%x,mvn_2=0X%x \n",mvn_1,mvn_2);
}

bool anti_rollback_enabled(void)
{
	const unsigned long cfg10 = readl(AO_SEC_SD_CFG10);
	return ( (cfg10 & (0x1<< 25)) ? true : false );
}


const char *amzn_target_device_name(void)
{
	static char target_name[16]={0};
	int i=0;
        if(strlen(CONFIG_DEVICE_PRODUCT) >= sizeof(target_name)) {
                return NULL;
        }
	strncpy(target_name,CONFIG_DEVICE_PRODUCT,strlen(CONFIG_DEVICE_PRODUCT));
	while(i<strlen(target_name)){
		target_name[i]=tolower(target_name[i]);
		i++;
	}
	printf("target_device_name is %s\n",target_name);
	return target_name;
}

int amzn_target_device_type(void)
{
	/* Is anti-rollback enabled? */
	if (anti_rollback_enabled() == true) {
		return AMZN_PRODUCTION_DEVICE;
	}
	else {
		return AMZN_ENGINEERING_DEVICE;
	}
}

bool amzn_target_is_lockdown()
{
	bool ret = true;
	/* Is this an engineering device? */
	if (amzn_target_device_type() == AMZN_ENGINEERING_DEVICE)
		ret = false;

	/* Are we un-locked? */
	if (amzn_target_is_unlocked())
		ret = false;

#if defined(UFBL_FEATURE_ONETIME_UNLOCK)
	if (amzn_target_is_onetime_unlocked())
		ret = false;
#endif

#if defined(UFBL_FEATURE_TEMP_UNLOCK)
	if (amzn_target_is_temp_unlocked())
		ret = false;
#endif

	return ret;
}
#if defined(UFBL_FEATURE_UNLOCK)
#define CHIPID_UPPER		(6)
#define CHIPID_LOWER		(7)
#define CHIPID_BUF_SIZE		(16)
#define HASH_BUF_SIZE		(32)

int amzn_get_unlock_code(unsigned char *code, unsigned int *len)
{
	sha256_context ctx;
	uint8_t buff[CHIPID_BUF_SIZE] = {0};
	uint8_t hash[HASH_BUF_SIZE] = {0};

	if (!code || !len || *len < (16 + 1))
		return -1;

	if (get_chip_id(&buff[0], sizeof(buff)))
		return -1;
	/**
	 * To sync with Amazon serial number from kernel's /proc/cpuinfo,
	 * the unlock_code is low 64 bit of sha256(SoC Chipid 128bits).
	 */
	sha256_starts(&ctx);
	sha256_update(&ctx, &buff[0], sizeof(buff));
	sha256_finish(&ctx, &hash[0]);
	u32 *hashcode = (u32 *) &hash[0];
	snprintf(code, CHIPID_BUF_SIZE+1, "%08x%08x",be32_to_cpu(hashcode[CHIPID_UPPER]),
			be32_to_cpu(hashcode[CHIPID_LOWER]));

	*len = 16;
	return 0;
}

const unsigned char *amzn_get_unlock_key(unsigned int *key_len)
{
	/* hadrian_unlock.pub.der */
	static const  unsigned char hadrian_unlock_pub_der[] = {
	0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
	0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00,
	0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xdc, 0xe0, 0x96,
	0xf6, 0x05, 0x68, 0xb2, 0xe3, 0x11, 0xac, 0xa8, 0x55, 0xde, 0xd8, 0xed,
	0xb7, 0xc7, 0x00, 0x4d, 0xeb, 0x23, 0xaf, 0x78, 0xe6, 0x1d, 0x7c, 0x73,
	0x6a, 0xc0, 0x72, 0x2a, 0xa7, 0xbd, 0x82, 0x7f, 0x47, 0xd1, 0xe8, 0x65,
	0x98, 0x91, 0x56, 0xa3, 0x50, 0xc9, 0x89, 0xb2, 0xb5, 0x9a, 0x7f, 0x81,
	0xf0, 0xc7, 0x18, 0x5b, 0x50, 0xf8, 0x2e, 0x57, 0xe0, 0x5f, 0xe6, 0xde,
	0x8a, 0xe8, 0x91, 0x76, 0xfd, 0x2d, 0x2c, 0x75, 0x30, 0xd0, 0x7f, 0x50,
	0xba, 0x37, 0xdc, 0x9e, 0xc9, 0x63, 0xe2, 0xdc, 0x96, 0x1f, 0x97, 0x17,
	0xf1, 0x19, 0x5f, 0xda, 0x44, 0xbd, 0x83, 0x55, 0x0c, 0xb5, 0xe6, 0x2f,
	0x27, 0xd9, 0xa0, 0x9d, 0x39, 0xf4, 0x20, 0x4d, 0x54, 0x83, 0xdc, 0xee,
	0xc0, 0x77, 0x9d, 0x65, 0xae, 0xb0, 0x50, 0x28, 0xe6, 0x29, 0x73, 0xbd,
	0x80, 0xa6, 0xf1, 0xfd, 0x80, 0x5c, 0x5f, 0x04, 0x0d, 0x75, 0x9b, 0x15,
	0xa1, 0xe9, 0xaa, 0x6c, 0x9c, 0xeb, 0x57, 0x21, 0x6f, 0x1d, 0x00, 0x3b,
	0x5a, 0x1a, 0xc9, 0x2b, 0x77, 0xfe, 0x0a, 0xce, 0xd4, 0x9e, 0x58, 0x8b,
	0xa4, 0xbb, 0x4c, 0x91, 0x80, 0x39, 0x0e, 0xa2, 0xd4, 0x6d, 0x72, 0x4a,
	0xf7, 0x86, 0x6d, 0x57, 0x2b, 0x68, 0x86, 0x92, 0x62, 0x8f, 0x70, 0xba,
	0x02, 0x22, 0x8f, 0x86, 0x99, 0x88, 0xcc, 0x59, 0xc2, 0x3c, 0xba, 0x2c,
	0x3b, 0xa1, 0x68, 0xaa, 0x0d, 0x49, 0xad, 0x7e, 0x4f, 0xce, 0xe2, 0x40,
	0x1c, 0x7d, 0x39, 0x60, 0x7e, 0xff, 0x3e, 0xa7, 0x9b, 0x06, 0x87, 0x46,
	0x1e, 0xfc, 0xcc, 0xb8, 0xe9, 0xb5, 0xba, 0x67, 0xdf, 0x5c, 0x16, 0xe8,
	0x27, 0x35, 0xf9, 0x11, 0x45, 0xa8, 0xb8, 0xe2, 0x4b, 0xd2, 0x23, 0xcd,
	0xc0, 0x61, 0x86, 0x0d, 0x46, 0xb6, 0x5b, 0x90, 0x4b, 0xd2, 0xe1, 0x49,
	0x67, 0x02, 0x03, 0x01, 0x00, 0x01
	};
	const int unlock_key_size = sizeof(hadrian_unlock_pub_der);
	if (!key_len)
		return NULL;

	*key_len = unlock_key_size;

	return hadrian_unlock_pub_der;
}

#ifdef UFBL_FEATURE_SECURE_FLASHING
static unsigned int get_random_number(int s)
{
	unsigned int seed = (unsigned int)get_timer(0) + s;
	seed ^= (seed << 13);
	seed ^= (seed >> 17);
	seed ^= (seed << 5);
	return seed;
}

int amzn_get_sec_flashing_code(unsigned char *code, unsigned int *len)
{
	static unsigned char sec_flashing_code[UNLOCK_CODE_LEN + 1] = {0};
	static unsigned char code_generated = 0;
	unsigned int unlock_code_len = UNLOCK_CODE_LEN;
	unsigned int rand1, rand2;

	if (!code || !len || *len < UNLOCK_CODE_LEN)
		return -1;

	if (!code_generated) {
		if(amzn_get_unlock_code(sec_flashing_code, &unlock_code_len)){
			return -1;
		}
		rand1 = get_random_number(0x1AB126);
		rand2 = get_random_number(rand1);
		sprintf(&sec_flashing_code[16], "%08x%08x", rand1, rand2);
		code_generated = 1;
	}
	memcpy(code, sec_flashing_code, UNLOCK_CODE_LEN);
	*len = UNLOCK_CODE_LEN;

	return 0;
}

const unsigned char *amzn_get_sec_flashing_root_pubkey(unsigned int *key_len)
{
	static const unsigned char root_key[] =
		"\x30\x82\x01\x22\x30\x0d\x06\x09\x2a\x86\x48\x86\xf7\x0d\x01\x01"
		"\x01\x05\x00\x03\x82\x01\x0f\x00\x30\x82\x01\x0a\x02\x82\x01\x01"
		"\x00\xb9\x20\xa0\x41\x68\x31\x06\xf4\x97\x32\x0d\xfc\x3a\x6c\x6a"
		"\xe9\x41\x6e\xfd\x57\x47\xd3\xdc\xef\xd7\x75\x24\x79\x33\x39\x71"
		"\x02\xd8\x72\x37\xd0\xdc\xc4\xed\x3d\x40\x6a\x20\xfa\xc7\x3f\x8e"
		"\x61\x81\xee\xff\x83\xaf\xbe\xb4\x51\xd8\xd2\x01\x42\xd5\x16\xda"
		"\x57\x12\x49\xaa\x3b\x50\xc7\x7e\xec\x47\x0b\x96\x31\xde\xa7\x4a"
		"\x9d\x7f\x7a\x44\xb3\xc2\x62\x8c\xa5\xe0\x0d\x48\xd9\x50\xa9\x69"
		"\xdc\x29\x42\x22\x33\xbb\xb0\x87\xfa\x51\x27\xd5\xf7\x11\x0c\x17"
		"\xbc\xe5\x5c\xa5\x60\x41\xd7\x07\xc0\xc2\x23\x65\x10\xb0\xc2\xa9"
		"\x12\xc4\x56\x80\xb9\xab\xf9\x1a\x89\xf0\x69\x98\xb3\xce\x9d\x22"
		"\x5a\xdf\xf2\x72\xf1\x93\x6e\xf9\xf4\x43\x87\xd0\x7c\xea\x21\x1b"
		"\xfd\xd9\xeb\xda\xba\x1c\x2a\x40\x3b\x3f\x22\xa8\xbc\x18\x5e\x85"
		"\x00\x84\xad\xb5\x88\xd1\x7f\x3d\x96\x73\x9a\x04\x78\xe5\x10\x5f"
		"\xdf\xed\x8c\xe2\x41\x8f\x21\x64\xf7\x54\xa7\xf2\xec\xc1\xe3\x09"
		"\x6e\x5f\xca\xdb\x78\x37\x29\xc0\x2a\xe1\xc5\x77\x32\xce\x5a\x0d"
		"\x4a\x30\xfd\x27\x8d\xa6\x11\x87\x62\xf6\x43\x44\xa7\x3a\xc6\x80"
		"\x03\xfc\x61\xfc\x6d\xae\xc5\x55\xcf\x5c\xee\x04\x24\x31\xb6\x7a"
		"\x5b\x02\x03\x01\x00\x01";
	const int key_size = sizeof(root_key);
	if (!key_len)
		return NULL;
	*key_len = key_size;
	return root_key;
}
#endif /* UFBL_FEATURE_SECURE_FLASHING */


#if defined(UFBL_FEATURE_ONETIME_UNLOCK)
int amzn_get_one_tu_code(unsigned char *code, unsigned int *len)
{
	static unsigned char code_generated = 0;
	static unsigned char one_tu_code[ONETIME_UNLOCK_CODE_LEN + 1] = {0};

	if (!code || !len || *len < ONETIME_UNLOCK_CODE_LEN)
		return -1;

	if (!code_generated) {
/**
 * Different SoC/Product may have different scheme to add entropy into PRNG.
 *
 */
#define ENTROPY_LEN (UNLOCK_CODE_LEN + 8)
		static unsigned char entropy[ENTROPY_LEN] = {0};
		unsigned int unlock_code_len = UNLOCK_CODE_LEN;
		if (amzn_get_unlock_code(entropy, &unlock_code_len)) {
			return -1;
		}
		sprintf(&entropy[unlock_code_len], "%08x", get_timer(0));
/**
 * amzn_get_onetime_random_number will return a binary string which is not readable.
 * The binary string cannot be returned via fastboot so using base64 encode it.
 */
// compute how many random bytes do we need so that the converted size is the target length
#define RANDOM_BYTES_SIZE (ONETIME_UNLOCK_CODE_LEN + 3) / 4 * 3
		uint8_t random_bytes[RANDOM_BYTES_SIZE] = {0};
		unsigned int out_len = sizeof(one_tu_code);

		if (amzn_get_onetime_random_number(entropy, strlen(entropy),
						random_bytes, sizeof(random_bytes)))
			return -1;

		if (amzn_onetime_unlock_b64_encode(random_bytes, sizeof(random_bytes),
						one_tu_code, &out_len)) {
			return -1;
		}
		code_generated = 1;
	}
	memcpy(code, one_tu_code, ONETIME_UNLOCK_CODE_LEN);
	*len = ONETIME_UNLOCK_CODE_LEN;
	return 0;
}

int amzn_get_onetime_unlock_root_pubkey(const unsigned char **key, unsigned int *key_len)
{
	static const unsigned char onetime_unlock_key[] = ONETIME_UNLOCK_KEY;
	const int onetime_unlock_key_size = sizeof(onetime_unlock_key);

	if (!key || !key_len)
		return -1;

	*key_len = onetime_unlock_key_size;
	*key = onetime_unlock_key;
	return 0;
}
#endif //UFBL_FEATURE_ONETIME_UNLOCK

#if defined(UFBL_FEATURE_TEMP_UNLOCK)

static struct boot_tag_temp_unlock g_temp_unlock_data;

void amzn_save_temp_unlock_data(void)
{
	int ret = 0;
	struct udevice *dev = NULL;
	struct tee_open_session_arg open_arg;
	struct tee_invoke_arg invoke_arg;
	const struct tee_optee_ta_uuid uuid = TA_TEMP_UNLOCK_UUID;
	struct tee_param params[1];
	const size_t buf_len = sizeof(struct boot_tag_temp_unlock);

	params[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
	params[0].u.memref.size = buf_len;

	dev = tee_find_device(NULL, NULL, NULL, NULL);
	if (dev == NULL) {
		printf("[Temp unlock] tee_find_device() failed");
		ret = -ENODEV;
		goto exit;
	}

	memset(&open_arg, 0, sizeof(open_arg));
	tee_optee_ta_uuid_to_octets(open_arg.uuid, &uuid);
	ret = tee_open_session(dev, &open_arg, 0, NULL);
	if (ret) {
		printf("[Temp unlock] tee_open_session() failed, ret = 0x%x\n", ret);
		goto exit;
	} else if (open_arg.ret != TEE_SUCCESS) {
		printf("tee_open_session() failed, ret = 0x%x, ret_origin=0x%x\n",
				open_arg.ret, open_arg.ret_origin);
		ret = open_arg.ret;
		goto exit;
	}

	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.session = open_arg.session;
	invoke_arg.func = CMD_GET_TEMP_UNLOCK_DATA;
	ret = tee_shm_alloc(dev, buf_len, TEE_SHM_ALLOC | TEE_SHM_REGISTER, &params[0].u.memref.shm);
	if (ret) {
		printf("[Temp unlock] tee_shm_alloc() failed, ret = 0x%x\n", ret);
		goto exit;
	}

	ret = tee_invoke_func(dev, &invoke_arg, sizeof(params) / sizeof(struct tee_param), params);
	if (ret) {
		printf("[Temp unlock] tee_invoke_func() failed, ret = 0x%x\n", ret);
		goto exit;
	} else if (invoke_arg.ret != TEE_SUCCESS) {
		printf("[Temp unlock] tee_invoke_func() failed, ret = 0x%x, origin = %d\n",
				invoke_arg.ret, invoke_arg.ret_origin);
		ret = invoke_arg.ret;
		goto exit;
	}

	memset(&g_temp_unlock_data, 0, buf_len);
	memcpy(&g_temp_unlock_data, params[0].u.memref.shm->addr, buf_len);

	// We only need CFG_AMZN_TEMP_UNLOCK_REBOOT_CNT temp unlock codes
	if (g_temp_unlock_data.temp_unlock_reboot_cnt > CFG_AMZN_TEMP_UNLOCK_REBOOT_CNT) {
		g_temp_unlock_data.temp_unlock_reboot_cnt = CFG_AMZN_TEMP_UNLOCK_REBOOT_CNT;
		memset(g_temp_unlock_data.temp_unlock_hmac + (CFG_AMZN_TEMP_UNLOCK_REBOOT_CNT * AMZN_TEMP_UNLOCK_HMAC_HASH_SIZE),
		0,
		sizeof(g_temp_unlock_data.temp_unlock_hmac) - (CFG_AMZN_TEMP_UNLOCK_REBOOT_CNT * AMZN_TEMP_UNLOCK_HMAC_HASH_SIZE));
	}

	printf("[Temp unlock] Save temp unlock data success!\n");

exit:
	tee_shm_free(params[0].u.memref.shm);
	if (dev != NULL)
		tee_close_session(dev, open_arg.session);
}

int amzn_get_temp_unlock_codes(unsigned char **codes, unsigned int *reboot_cnt)
{
	if (codes == NULL || reboot_cnt == NULL)
		return -1;

	if (g_temp_unlock_data.magic != AMZN_TEMP_UNLOCK_BOOT_TAG_MAGIC) {
		printf("Warning: No valid temp unlock codes from sboot!\n");
		*reboot_cnt = 0;
		*codes = NULL;
	return -1;
	}

	*reboot_cnt = g_temp_unlock_data.temp_unlock_reboot_cnt;
	*codes = g_temp_unlock_data.temp_unlock_hmac;
	return 0;
}

int amzn_get_temp_unlock_root_pubkey(unsigned char **key, unsigned int *key_len)
{
	// temp-unlock and one-time-unlock share the same root public key
	return amzn_get_onetime_unlock_root_pubkey(key, key_len);
}

#endif //UFBL_FEATURE_TEMP_UNLOCK

#endif


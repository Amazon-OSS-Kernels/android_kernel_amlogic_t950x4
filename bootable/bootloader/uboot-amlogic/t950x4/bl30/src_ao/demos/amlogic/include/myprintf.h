// SPDX-License-Identifier: (GPL-2.0+ OR MIT)


#ifndef __PRINTF_H
#define __PRINTF_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdarg.h>		/* For va_list */
#include <stdint.h>

#include "common.h"
#include "FreeRTOSConfig.h"
#define KERN_ERROR		1
#define KERN_WARNING	2
#define KERN_DEBUG		3
#define KERN_INFO		4

#ifndef TAG
#define TAG
#endif

#if CONFIG_LOGLEVEL_DEFAULT >= 1
#define AGLOGE(format, ...)		LOGE(KERN_ERROR, TAG format, ##__VA_ARGS__)
#else
#define AGLOGE(format, ...)
#endif

#if CONFIG_LOGLEVEL_DEFAULT >= 2
#define AGLOGW(format, ...)		LOGW(KERN_WARNING, TAG format, ##__VA_ARGS__)
#else
#define AGLOGW(format, ...)
#endif

#if CONFIG_LOGLEVEL_DEFAULT >= 3
#define AGLOGD(format, ...)		LOGD(KERN_DEBUG, TAG format, ##__VA_ARGS__)
#else
#define AGLOGD(format, ...)
#endif

//#define LOGE(KERN_ERROR, format, ...)         aml_log_out(KERN_ERROR, format, ##__VA_ARGS__)
//#define LOGW(KERN_WARNING, format, ...)               aml_log_out(KERN_WARNING, format, ##__VA_ARGS__)
//#define LOGD(KERN_DEBUG, format, ...)         aml_log_out(KERN_DEBUG, format, ##__VA_ARGS__)
#if 0
#define LOGE(LOG_TAG, format, ...)		aml_log_out(KERN_ERROR, LOG_TAG"<E>" format, ##__VA_ARGS__)
#define LOGW(LOG_TAG, format, ...)		aml_log_out(KERN_WARNING, LOG_TAG"<W>" format, ##__VA_ARGS__)
#define LOGD(LOG_TAG, format, ...)		aml_log_out(KERN_DEBUG, LOG_TAG"<D>" format, ##__VA_ARGS__)
#define LOGI(LOG_TAG, format, ...)		aml_log_out(KERN_INFO, LOG_TAG"<I>" format, ##__VA_ARGS__)
#endif

	int vfnprintf(int (*addchar)(void *context, int c), void *context,
		      const char *format, va_list args);

	int sPrintf_ext(char *str, size_t size, const char *format,
			va_list args);
	int iprintf(const char *fmt, ...);
	int sPrintf(char *str, size_t size, const char *fmt, ...);
	int syslog_buf_init(void);
//	int aml_log_out(int level, const char *fmt, ...);
	int iprint_string(char *str);

#ifndef MYPRINTF_OVERRIDE_LIBC
#define MYPRINTF_OVERRIDE_LIBC	1
#endif

#if MYPRINTF_OVERRIDE_LIBC
#define printf iprintf
#endif

#ifdef __cplusplus
}
#endif
#endif				/* __CROS_EC_PRINTF_H */

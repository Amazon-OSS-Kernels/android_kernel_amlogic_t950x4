// SPDX-License-Identifier: (GPL-2.0+ OR MIT)

/* wake up reason*/
#define	UDEFINED_WAKEUP	0
#define REMOTE_CUS5_WAKEUP 1
#define	REMOTE_WAKEUP		2
#define	RTC_WAKEUP			3
#define	BT_WAKEUP			4
#define	WIFI_WAKEUP			5
#define	POWER_KEY_WAKEUP	6
#define        AUTO_WAKEUP                     7
#define CEC_WAKEUP		8
#define	REMOTE_CUS_WAKEUP		9
#define ETH_PMT_WAKEUP      10
#define REMOTE_CUS6_WAKEUP		11
#define ETH_PHY_GPIO    12
#define REMOTE_CUS1_WAKEUP 13
#define REMOTE_CUS2_WAKEUP 14
#define REMOTE_CUS3_WAKEUP 0x0f

#define STR_QUEUE_LENGTH    32
#define STR_QUEUE_ITEM_SIZE 4

typedef struct {
	char* name;
} WakeUp_Reason;

extern void create_str_task(void);
extern void STR_Start_Sem_Give_FromISR(void);
extern void STR_Start_Sem_Give(void);
extern void STR_Wakeup_src_Queue_Send_FromISR(uint32_t *src);
extern void STR_Wakeup_src_Queue_Send(uint32_t *src);
extern void *xMboxSuspend_Sem(void *msg);


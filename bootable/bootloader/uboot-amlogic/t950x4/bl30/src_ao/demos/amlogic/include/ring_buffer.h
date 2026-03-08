/*
 * Ring buffer for BL30 log share to kernel
 *
 * Copyright (C) 2021-2022 amlogic
 *
 */

/*

Ring Buffer implementation

1)
head points to first available data byte
tail points to first available byte space
2)
if head==tail && len==0      // empty
if head==tail && len==size   // full
3)
if head>=size   head%=size
if tail>=size   tail%=size

*/
#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include "soc.h"

//#define BL30MSG_BUF_BASE (0x40000000 + 0x1000)
//#define BL30MSG_BUF_BASE (0x2e000000 + 0x1000)
//#define BL30MSG_BUF_BASE (0x10000000 + (64*1024))
#define BL30MSG_BUF_BASE (0xfffc0000 + (30*1024))   /*(0xfffc0000 + (30*1024))=0xfffc7800*/
#define BL30MSG_BUF_SIZE (2*1024)
#define BL30MSG_LEN 100

#define MAGIC  0x11223344

#ifndef NULL
#define NULL   ((void *)0)
#endif

#define RISC_V_CPU    0
#define ARM_CPU       1

/*
1) req is used for request lock
2) turn is flag for who should own the lock when they request at same time.
*/
typedef struct amp_lock_s{
	volatile int turn;
	volatile int req[2];
}amp_lock;

struct ring_buffer {
	unsigned int magic;      // magic number
	amp_lock     lock;           // exclusive lock (implement future)
	unsigned int size;       // total size of ring buffer data
	unsigned int head;       // for read data offset, arm(kernel) maintain, kernel move it
	unsigned int tail;       // for write data offset, risc-v maintain it
	unsigned int len;        // available log data in ring buffer
	char data[4];            // log buffer payload start
};

extern int lock(void);
extern int unlock(void);
extern struct ring_buffer * init_ring_buffer(unsigned char * buffer, unsigned int size);
extern unsigned int read_ring_buffer(struct ring_buffer *rb, unsigned char *buffer, unsigned int size);
extern unsigned int write_ring_buffer(struct ring_buffer *rb, unsigned char *buffer, unsigned int size);
extern void reset_ring_buffer(struct ring_buffer *rb);

#endif

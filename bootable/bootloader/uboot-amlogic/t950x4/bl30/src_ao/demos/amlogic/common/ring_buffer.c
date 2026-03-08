/* * Ring buffer for BL30 log share to kernel
 * Copyright (C) 2021-2022 amlogic
 *
 */
#include "FreeRTOS.h"
#include "task.h"
#include "ring_buffer.h"
#define LOCK 1

static int amp_lock_init(amp_lock *lock);
int amp_lock_obtain(amp_lock *lock, int self);
int amp_lock_release(amp_lock *lock, int self);

static void Lock_EnterCritical(void)
{
        taskENTER_CRITICAL();
}

static void Lock_ExitCritical(void)
{
        taskEXIT_CRITICAL();
}

static int amp_lock_init(amp_lock *lock)
{
	//memset(lock,0,sizeof(amp_lock));
	lock->turn = 0;
	lock->req[0] = lock->req[1] = 0;
	return 0;
}

int amp_lock_obtain(amp_lock *lock, int self)
{
#if LOCK
	int other = 1 - self;

    // disable interrupt here
	Lock_EnterCritical();

	lock->req[self] = 1;
	if (lock->req[other])
	{
		lock->req[self] = 0;
		while (lock->turn != self);
		lock->req[self] = 1;
		while (lock->req[other]);
	}
#endif
	return 0;
}
int amp_lock_release(amp_lock *lock, int self)
{
#if LOCK
	int other = 1 - self;
	lock->turn = other;
	lock->req[self] = 0;

    // enable interrupt here
	Lock_ExitCritical();
#endif
	return 0;
}

int lock(void)
{
	return 0;
}

int unlock(void)
{
	return 0;
}

struct ring_buffer * init_ring_buffer(unsigned char * buffer, unsigned int size)
{
	struct ring_buffer * b = (struct ring_buffer *)buffer;

	if (!b || size < sizeof (struct ring_buffer))
		return NULL;
	if (b->magic == MAGIC)
		return b;

	b->magic = MAGIC;
	b->size = size - sizeof (struct ring_buffer) + 4;
	b->head = 0;
	b->tail = 0;
	b->len  = 0;
	amp_lock_init(&b->lock);
	return b;
}

unsigned int read_ring_buffer(struct ring_buffer *rb, unsigned char *buffer, unsigned int size)
{
	unsigned int count,rlen;

	if (!rb || rb->magic != MAGIC || rb->len == 0)
		return 0;

	count = 0;
	amp_lock_obtain(&rb->lock, RISC_V_CPU);
	rlen = size < rb->len ? size : rb->len;
	//lock();
	while (rlen--) {
		buffer[count++] = rb->data[rb->head++];
		rb->head %= rb->size;
		rb->len--;
	}
	//unlock();
	amp_lock_release(&rb->lock, RISC_V_CPU);
	return count;
}

unsigned int write_ring_buffer(struct ring_buffer *rb, unsigned char *buffer, unsigned int size)
{
	unsigned int count,wlen;

	if (!rb || rb->magic != MAGIC)
		return 0;

	count = 0;
	amp_lock_obtain(&rb->lock, RISC_V_CPU);
	//lock();
	wlen = size;
	while (wlen--) {
		rb->data[rb->tail++] = buffer[count++];
		rb->tail %= rb->size;
		rb->len++;
	}

	/* New data will overwrite the old data */
	if (rb->len > rb->size) {
		rb->head = rb->tail;
		rb->len = rb->size;
	}
	//unlock();
	amp_lock_release(&rb->lock, RISC_V_CPU);

	return count;
}

void reset_ring_buffer(struct ring_buffer *rb)
{
	if (!rb || rb->magic != MAGIC)
		return;

	rb->head = 0;
	rb->tail = 0;
	rb->len  = 0;
	amp_lock_init(&rb->lock);
}

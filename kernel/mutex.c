/*
 * mutex.c
 * Fairly self-explanatory.
 */
#include <asm/interrupts.h>

#include <stdint.h>
#include <stddef.h>

#include <kernel/mutex.h>
#include <kernel/proc.h>

void
mutex_init(mutex *m)
{
	*m = MUTEX_UNLOCKED;
}

void
mutex_lock(mutex *m)
{
	disable_intr();
	while (*m == MUTEX_LOCKED)
		sleep(m);
	*m = MUTEX_LOCKED;
	enable_intr();
}

void
mutex_unlock(mutex *m)
{
	*m = MUTEX_UNLOCKED;
	wakeup(m);
}

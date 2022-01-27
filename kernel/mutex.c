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
	m->lock = MUTEX_UNLOCKED;
	m->owner = NULL;
}

void
mutex_lock(mutex *m)
{
	if (m->owner == NULL && m->lock == MUTEX_LOCKED) {
		printk("mutex %p is corrupt: it has no owner but it is locked\r\n");
		panic("mutex_lock: Corrupt mutex\r\n");
	}
	disable_intr();
	if (m->owner != current_process) {
		while (m->lock == MUTEX_LOCKED)
			sleep(m, PROC_SLEEP_INTERRUPTIBLE);
	}
	m->lock = MUTEX_LOCKED;
	m->owner = current_process;
	/*	restore_intr_state(); */
	enable_intr();
}

void
mutex_unlock(mutex *m)
{
	disable_intr();
	m->lock = MUTEX_UNLOCKED;
	m->owner = NULL;
	wakeup(m, WAKEUP_SWITCH);
	/* restore_intr_state(); */
	enable_intr();
}

int
mutex_locked(mutex *m)
{
	return m->lock == MUTEX_LOCKED && m->owner != current_process;
}

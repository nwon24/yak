#ifndef MUTEX_H
#define MUTEX_H

#define MUTEX_UNLOCKED	0
#define MUTEX_LOCKED	1

struct process;

typedef int mutex;

void mutex_init(mutex *mutex);
void mutex_lock(mutex *mutex);
void mutex_unlock(mutex *mutex);

#endif /* MUTEX_H */

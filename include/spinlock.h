#ifndef MNTHREAD_SPINLOCK_H
#define MNTHREAD_SPINLOCK_H

#include <stdatomic.h>

/*
 * spinlock.h -- Simple spinlock using C11 atomics.
 *
 * Used to protect the shared ready queue in the M:N model (Step 4).
 * For the 1:N cooperative model (Step 3) this is not needed since
 * there is only one kernel thread.
 *
 * This is a test-and-set spinlock. It burns CPU while waiting.
 * Acceptable for a learning project; production code would use
 * futexes or pthread_mutex_t.
 */

typedef struct {
    atomic_flag flag;
} spinlock_t;

/* Static initializer: spinlock_t lock = SPINLOCK_INIT; */
#define SPINLOCK_INIT { .flag = ATOMIC_FLAG_INIT }

/* Initialize a spinlock at runtime. */
void spinlock_init(spinlock_t *lock);

/* Acquire the lock. Spins until successful. */
void spinlock_lock(spinlock_t *lock);

/* Release the lock. Caller must hold it. */
void spinlock_unlock(spinlock_t *lock);

#endif /* MNTHREAD_SPINLOCK_H */

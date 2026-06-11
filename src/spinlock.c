#include "spinlock.h"
#include "stdatomic.h"

void spinlock_init(spinlock_t *lock)
{
    atomic_flag_clear(&lock->flag);
}

void spinlock_lock(spinlock_t *lock)
{
    /*
     * TODO: Spin on atomic_flag_test_and_set until we acquire.
     *
     * Use memory_order_acquire on the test-and-set.
     * Optionally add a pause/yield hint in the spin loop
     * to reduce bus contention (e.g., __builtin_ia32_pause()).
     */

    // spin-wait while mutex is taken
    while(atomic_flag_test_and_set_explicit(&lock->flag, memory_order_acquire) == 1);

    // mutex aquired above
}

void spinlock_unlock(spinlock_t *lock)
{
    /*
     * TODO: Clear the flag with memory_order_release.
     */
    atomic_flag_clear_explicit(&lock->flag, memory_order_release);
}

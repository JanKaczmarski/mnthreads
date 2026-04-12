#include "spinlock.h"

void spinlock_init(spinlock_t *lock)
{
    /* TODO: Clear the atomic flag. */
    (void)lock;
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
    (void)lock;
}

void spinlock_unlock(spinlock_t *lock)
{
    /*
     * TODO: Clear the flag with memory_order_release.
     */
    (void)lock;
}

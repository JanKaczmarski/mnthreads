/*
 * test_mn.c -- Step 4: M:N scheduling with multiple kernel threads.
 *
 * 16 user threads distributed across 4 kernel workers.
 * Validates that user threads migrate between kernel threads
 * and that the spinlock-protected queue doesn't corrupt.
 */

#include "mnthread.h"
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

#define NUM_USER_THREADS  16
#define NUM_WORKERS        4
#define YIELDS_PER_THREAD 10000

static atomic_int total_yields = 0;

static void worker_func(void)
{
    /*
     * TODO:
     *  1. Get own thread ID.
     *  2. Loop YIELDS_PER_THREAD times:
     *       a. atomic_fetch_add(&total_yields, 1)
     *       b. Optionally: printf("[UT %d] on kernel thread %lu\n",
     *                              my_id, (unsigned long)pthread_self());
     *          (Enable this for debugging; disable for stress test
     *          since printf is slow and contended.)
     *       c. mnthread_yield();
     */
}

int main(void)
{
    printf("=== Step 4: M:N scheduling ===\n");
    printf("  %d user threads, %d kernel workers\n",
           NUM_USER_THREADS, NUM_WORKERS);

    mnthread_init(NUM_WORKERS);

    for (int i = 0; i < NUM_USER_THREADS; i++) {
        int id = mnthread_spawn(worker_func);
        if (id < 0) {
            fprintf(stderr, "Failed to spawn thread %d\n", i);
            return 1;
        }
    }

    /*
     * TODO: Wait for all threads to finish.
     * In M:N mode, the workers are running in the background.
     * Use mnthread_join on each thread ID, or implement a
     * barrier/wait-all mechanism.
     */
    for (int i = 0; i < NUM_USER_THREADS; i++) {
        mnthread_join(i);
    }

    int expected = NUM_USER_THREADS * YIELDS_PER_THREAD;
    int actual   = atomic_load(&total_yields);

    printf("Total yields: %d (expected %d)\n", actual, expected);

    if (actual == expected) {
        printf("PASS\n");
    } else {
        printf("FAIL: yield count mismatch\n");
        return 1;
    }

    mnthread_shutdown();
    return 0;
}

/*
 * test_join.c -- Step 5: Thread join.
 *
 * Validates that mnthread_join blocks until the target thread
 * finishes, and that main doesn't exit prematurely.
 */

#include "mnthread.h"
#include <stdio.h>

#define NUM_THREADS 8

static int finished_flags[NUM_THREADS];

static void worker_func(void)
{
    /*
     * TODO:
     *  1. Get own thread ID.
     *  2. Do some work (e.g., yield a few times to simulate work).
     *  3. Set finished_flags[my_id] = 1.
     *  4. Return (trampoline handles exit).
     */
}

int main(void)
{
    printf("=== Step 5: Thread join ===\n");

    /* Test with M:N mode. */
    mnthread_init(4);

    int ids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        finished_flags[i] = 0;
        ids[i] = mnthread_spawn(worker_func);
        if (ids[i] < 0) {
            fprintf(stderr, "Failed to spawn thread %d\n", i);
            return 1;
        }
    }

    /* Join all threads. Main should block until each is done. */
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("[main] Joining thread %d...\n", ids[i]);
        int ret = mnthread_join(ids[i]);
        if (ret != 0) {
            fprintf(stderr, "mnthread_join(%d) returned error\n", ids[i]);
            return 1;
        }
        printf("[main] Thread %d joined.\n", ids[i]);
    }

    /* Verify all threads actually ran. */
    int pass = 1;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (!finished_flags[i]) {
            fprintf(stderr, "Thread %d did not set its flag!\n", i);
            pass = 0;
        }
    }

    if (pass) {
        printf("PASS: All threads joined successfully.\n");
    } else {
        printf("FAIL: Some threads did not complete.\n");
    }

    mnthread_shutdown();
    return pass ? 0 : 1;
}

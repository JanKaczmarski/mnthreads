/*
 * test_coop.c -- Step 3: Cooperative 1:N scheduling.
 *
 * Multiple user threads run on a single OS thread by calling
 * mnthread_yield(). Validates round-robin interleaving.
 */

#include "mnthread.h"
#include <stdio.h>

#define NUM_THREADS  5
#define YIELDS_PER_THREAD 20000

static int run_counts[NUM_THREADS];

static void worker_func(void)
{
    /*
     * Problem: how does each thread know its own index?
     *
     * Options:
     *   a) Use scheduler_current()->id and index into run_counts.
     *   b) Use thread-local-like storage on the TCB (e.g., a
     *      user-data void* field you could add to tcb_t).
     *
     * For now, just use the thread ID from the scheduler.
     */

    /* TODO: Retrieve own thread ID, then loop:
     *   for (int i = 0; i < YIELDS_PER_THREAD; i++) {
     *       run_counts[my_id]++;
     *       mnthread_yield();
     *   }
     */
}

int main(void)
{
    printf("=== Step 3: Cooperative 1:N scheduling ===\n");

    mnthread_init(0);  /* Cooperative mode: 0 workers */

    for (int i = 0; i < NUM_THREADS; i++) {
        run_counts[i] = 0;
        int id = mnthread_spawn(worker_func);
        if (id < 0) {
            fprintf(stderr, "Failed to spawn thread %d\n", i);
            return 1;
        }
        printf("Spawned user thread %d\n", id);
    }

    /*
     * TODO: Run the scheduler until all threads finish.
     * In cooperative mode, you might need a scheduler_run_until_done()
     * function, or have main yield in a loop until the queue is empty.
     */

    printf("\nResults:\n");
    int pass = 1;
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("  Thread %d: ran %d times\n", i, run_counts[i]);
        if (run_counts[i] != YIELDS_PER_THREAD) {
            pass = 0;
        }
    }

    if (pass) {
        printf("PASS: All threads completed %d iterations.\n",
               YIELDS_PER_THREAD);
    } else {
        printf("FAIL: Iteration counts do not match.\n");
    }

    mnthread_shutdown();
    return pass ? 0 : 1;
}

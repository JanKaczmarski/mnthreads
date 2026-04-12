/*
 * test_create.c -- Step 2: TCB creation and trampoline.
 *
 * Validates that tcb_create sets up a proper initial stack frame,
 * and that switching into it runs the function, which then falls
 * into the trampoline and calls thread_exit cleanly.
 */

#include "mnthread.h"
#include <stdio.h>

static int thread_ran = 0;

static void test_func(void)
{
    printf("[test_func] Hello from user thread!\n");
    thread_ran = 1;
    /* Function returns -> trampoline should call thread_exit(). */
}

int main(void)
{
    printf("=== Step 2: TCB creation and trampoline ===\n");

    /* Use cooperative mode (0 workers) for this test. */
    mnthread_init(0);

    int id = mnthread_spawn(test_func);
    if (id < 0) {
        fprintf(stderr, "mnthread_spawn not yet implemented\n");
        return 1;
    }

    printf("[main] Spawned thread %d\n", id);

    /*
     * In cooperative mode, main needs to kick the scheduler.
     * This will switch to test_func, which runs, returns into
     * the trampoline, calls thread_exit, and the scheduler
     * should switch back to main.
     *
     * TODO: You may need a scheduler_run() or equivalent here
     * that gives the scheduler control until all threads finish.
     * For now, a single yield from main's perspective may suffice.
     */
    mnthread_yield();

    if (thread_ran) {
        printf("PASS: Thread executed and exited cleanly.\n");
    } else {
        printf("FAIL: Thread did not run.\n");
        return 1;
    }

    mnthread_shutdown();
    return 0;
}

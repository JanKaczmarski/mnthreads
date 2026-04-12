#include "mnthread.h"
#include "tcb.h"
#include "scheduler.h"
#include "worker.h"

#include <stdio.h>
#include <stddef.h>

/*
 * Registry of all created threads, indexed by ID.
 * Needed so mnthread_join(id) can look up the TCB.
 * Hardcoded size -- increase if needed.
 */
#define MAX_THREADS 256
static tcb_t *thread_registry[MAX_THREADS];
static int     thread_count = 0;

void mnthread_init(int num_workers)
{
    /*
     * TODO:
     *  1. scheduler_init()
     *  2. Clear thread_registry.
     *  3. If num_workers > 0: workers_start(num_workers)
     */
    (void)num_workers;
}

int mnthread_spawn(void (*func)(void))
{
    /*
     * TODO:
     *  1. tcb_t *t = tcb_create(func)
     *  2. If t == NULL: return -1
     *  3. thread_registry[t->id] = t
     *  4. thread_count++
     *  5. scheduler_enqueue(t)
     *  6. return t->id
     */
    (void)func;
    return -1;
}

void mnthread_yield(void)
{
    /*
     * TODO: scheduler_yield()
     */
}

void mnthread_exit(void)
{
    /*
     * TODO: scheduler_thread_exit()
     */
}

int mnthread_join(int thread_id)
{
    /*
     * TODO:
     *  1. If thread_id < 0 || thread_id >= MAX_THREADS: return -1
     *  2. tcb_t *target = thread_registry[thread_id]
     *  3. If target == NULL: return -1
     *  4. scheduler_join(target)
     *  5. return 0
     */
    (void)thread_id;
    return -1;
}

void mnthread_shutdown(void)
{
    /*
     * TODO:
     *  1. scheduler_shutdown()
     *  2. Clear thread_registry.
     *  3. thread_count = 0
     */
}

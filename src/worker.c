#include "worker.h"
#include "scheduler.h"
#include "context.h"
#include "tcb.h"

#include <stdio.h>
#include <stddef.h>

/* Array of worker structs. Sized statically for simplicity. */
static worker_t workers[MAX_WORKERS];
static int      worker_count = 0;

/* Thread-local pointer to this kernel thread's worker struct. */
static __thread worker_t *tls_worker = NULL;

void *worker_loop(void *arg)
{
    /*
     * TODO:
     *  1. worker_t *self = (worker_t *)arg;
     *  2. tls_worker = self;
     *
     *  3. while (self->running):
     *       a. tcb_t *t = scheduler_dequeue();
     *       b. If t == NULL:
     *            - Queue is empty. Either spin, or (better) sleep
     *              briefly and retry. A condition variable would be
     *              ideal here but a usleep(100) is fine for now.
     *            - continue;
     *       c. scheduler_set_current(t);
     *       d. switch_context(&self->sp, t->sp);
     *          // -- we return here when the user thread yields
     *          //    or exits --
     *       e. tcb_t *finished = scheduler_current();
     *          // scheduler_current() may be NULL if thread exited
     *          // or may still point to a thread that yielded.
     *       f. If the previous thread is FINISHED:
     *            tcb_destroy(finished);
     *          // Safe now because we're on the worker's own stack.
     *
     *  4. return NULL;
     */

    (void)arg;
    return NULL;
}

void workers_start(int num_workers)
{
    /*
     * TODO:
     *  1. worker_count = num_workers (clamp to MAX_WORKERS).
     *  2. For each worker i:
     *       a. workers[i].id = i
     *       b. workers[i].sp = NULL  (will be set on first switch)
     *       c. workers[i].running = 1
     *       d. pthread_create(&workers[i].pthread, NULL,
     *                         worker_loop, &workers[i])
     */
    (void)num_workers;
}

void workers_stop(void)
{
    /*
     * TODO:
     *  1. For each worker: workers[i].running = 0
     *  2. For each worker: pthread_join(workers[i].pthread, NULL)
     *  3. worker_count = 0
     */
}

worker_t *worker_self(void)
{
    return tls_worker;
}

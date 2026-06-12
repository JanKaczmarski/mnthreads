#include "scheduler.h"
#include "tcb.h"
#include "context.h"
#include "spinlock.h"
#include "queue.h"

#include <stddef.h>
#include <stdio.h>

/* ---------------------------------------------------------------
 * Global scheduler state
 * --------------------------------------------------------------- */

/* The shared ready queue. */
static thread_queue_t ready_queue;

/* Protects ready_queue in M:N mode (Step 4+). */
static spinlock_t     queue_lock;

/*
 * Thread-local pointer to the TCB currently running on this
 * kernel thread / worker. In Step 3 (single-threaded), this
 * is just a global.
 */
static __thread tcb_t *current_tcb = NULL;

/*
 * Number of active workers. 0 means cooperative mode (Step 3).
 */
static int num_workers_active = 0;

/* ---------------------------------------------------------------
 * Lifecycle
 * --------------------------------------------------------------- */

void scheduler_init(void)
{
    queue_init(&ready_queue);
    spinlock_init(&queue_lock);
    current_tcb = NULL;
}

void scheduler_shutdown(void)
{
    /*
     * TODO:
     *  1. If workers are active, call workers_stop().  - after Step 3
     *  2. Drain any remaining TCBs from the ready queue
     *     and call tcb_destroy on each.
     *  3. Any other global cleanup.
     */
    // workers_stop();

    spinlock_lock(&queue_lock);

    while (!queue_empty(&ready_queue)) {
        tcb_t* tcb = queue_pop(&ready_queue);
        tcb_destroy(tcb);
    }

    spinlock_unlock(&queue_lock);
}

/* ---------------------------------------------------------------
 * Queue operations
 * --------------------------------------------------------------- */

void scheduler_enqueue(tcb_t *t)
{
    spinlock_lock(&queue_lock);

    t->state = THREAD_READY;
    queue_push(&ready_queue, t);

    spinlock_unlock(&queue_lock);
}

tcb_t *scheduler_dequeue(void)
{
    spinlock_lock(&queue_lock);

    tcb_t *t = queue_pop(&ready_queue);
    if (t != NULL) {
        t->state = THREAD_RUNNING;
    }
    scheduler_set_current(t);

    spinlock_unlock(&queue_lock);

    return t;
}

/* ---------------------------------------------------------------
 * Current-thread tracking
 * --------------------------------------------------------------- */

tcb_t *scheduler_current(void)
{
    return current_tcb;
}

void scheduler_set_current(tcb_t *t)
{
    current_tcb = t;
}

/* ---------------------------------------------------------------
 * Scheduling actions
 * --------------------------------------------------------------- */

void scheduler_yield(void)
{
    /*
     * TODO (Step 4 -- M:N with workers):
     *  1. tcb_t *self = current_tcb
     *  2. scheduler_enqueue(self)  // put back in queue
     *  3. current_tcb = NULL
     *  4. worker_t *w = worker_self()
     *  5. switch_context(&self->sp, w->sp)
     *     // Returns into the worker loop, which will dequeue
     *     // the next thread.
     *
     * CRITICAL (Step 4): The yield race condition.
     *   Between enqueue and switch_context, another Worker could
     *   dequeue this TCB and start running it while WE are still
     *   on its stack finishing the switch. Solutions:
     *     a) Hold the queue lock across enqueue + switch_context.
     *     b) Use a TCB state machine: RUNNING -> YIELDING -> READY
     *        with careful atomic transitions.
     *   Choose one and implement it; do not leave this unhandled.
     */


    tcb_t *prev = current_tcb;
    scheduler_enqueue(prev);

    // TODO: what to switch to if next == NULL?
    tcb_t *next = scheduler_dequeue();
    current_tcb = next;

    switch_context(&prev->sp, next->sp);
}

void scheduler_thread_exit(void)
{
    /*
     * TODO:
     *  1. tcb_t *self = current_tcb
     *  2. self->state = THREAD_FINISHED
     *  3. If self->joiner != NULL:
     *       scheduler_enqueue(self->joiner)   // wake the joiner
     *  4. current_tcb = NULL
     *  5. Switch away:
     *       - Step 3: dequeue next thread and switch to it, or
     *                 switch to idle/scheduler context if queue empty.
     *       - Step 4: switch back to worker_self()->sp.
     *  6. The worker/scheduler context is responsible for calling
     *     tcb_destroy(self) AFTER the switch completes.
     *
     * NOTE: Do NOT call tcb_destroy here. We are still running
     * on self's stack. Freeing it now is undefined behavior.
     */

    tcb_t *self = current_tcb;
    if (self == NULL) {
        return;
    }
    
    self->state = THREAD_FINISHED;
    if (self->joiner != NULL) {
        scheduler_enqueue(self->joiner);
    }

    tcb_t *next = scheduler_dequeue();
    if (next == NULL) {
        next = NULL; // TODO: switch to scheduler context here
    }
    current_tcb = next;

    switch_context(&self->sp, next->sp); // TODO: why not tcb destroy after context switch?
}

void scheduler_join(tcb_t *target)
{
    /*
     * TODO (Step 5):
     *  1. If target->state == THREAD_FINISHED: return immediately.
     *  2. tcb_t *self = current_tcb
     *  3. target->joiner = self
     *  4. self->state = THREAD_BLOCKED
     *  5. Switch away (do NOT enqueue self -- we're blocked, not ready).
     *       - Step 3: dequeue next thread and switch to it.
     *       - Step 4: switch back to worker context.
     *  6. When target finishes, scheduler_thread_exit() will
     *     enqueue us back and we'll eventually be scheduled again.
     *     Execution resumes here after the switch.
     *
     * RACE CONDITION: target could finish between step 1 and
     * step 3. Use a lock or atomic state check to handle this.
     */
    (void)target;
}

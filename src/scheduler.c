#include "scheduler.h"
#include "scheduler_struct.h"
#include "tcb.h"
#include "context.h"
#include "spinlock.h"
#include "queue.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Default scheduler used by entities that don't have access to scheduler creation (like tcb.c)
static scheduler_t *default_scheduler = NULL;

/* ---------------------------------------------------------------
 * Lifecycle
 * --------------------------------------------------------------- */

scheduler_t *scheduler_default(void) {
    assert(default_scheduler != NULL && "scheduler_default called with default scheduler being NULL (not set)");
    return default_scheduler;
}

void scheduler_set_default(scheduler_t *s) {
    assert(s != NULL && "cannot set NULL as default scheduler");
    assert(default_scheduler == NULL && "default scheduler already initialized");
    default_scheduler = s;
}

scheduler_t *scheduler_create(void)
{
    struct scheduler *s = malloc(sizeof(struct scheduler));
    if (s == NULL) return NULL;

    queue_init(&s->ready_queue);
    spinlock_init(&s->queue_lock);
    s->current_tcb = NULL;
    s->num_workers_active = 0; // default for Stage 3 (1 kernel worker)
    
    return s;
}

void scheduler_destroy(scheduler_t *s)
{
    /*
     * TODO: If workers are active, call workers_stop().  - after Step 3
     * After Step 3 (multiple workers) add a spinlock
     */
    // workers_stop();

    while (!queue_empty(&s->ready_queue)) {
        tcb_t* tcb = queue_pop(&s->ready_queue);
        tcb_destroy(tcb);
    }

    free(s);
}

/* ---------------------------------------------------------------
 * Queue operations
 * --------------------------------------------------------------- */

void scheduler_enqueue(scheduler_t *s, tcb_t *t)
{
    t->state = THREAD_READY;
    queue_push(&s->ready_queue, t);
}

tcb_t *scheduler_dequeue(scheduler_t *s)
{
    tcb_t *t = queue_pop(&s->ready_queue);
    if (t != NULL) {
        t->state = THREAD_RUNNING;
    }
    s->current_tcb = t;

    return t;
}

/* ---------------------------------------------------------------
 * Current-thread tracking
 * --------------------------------------------------------------- */

tcb_t *scheduler_current(scheduler_t *s)
{
    return s->current_tcb;
}

/* ---------------------------------------------------------------
 * Scheduling actions
 * --------------------------------------------------------------- */

void scheduler_yield(scheduler_t *s)
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


    tcb_t *prev = s->current_tcb;
    scheduler_enqueue(s, prev);

    // TODO: what to switch to if next == NULL?
    tcb_t *next = scheduler_dequeue(s);
    s->current_tcb = next;

    switch_context(&prev->sp, next->sp);
}

void scheduler_thread_exit(scheduler_t *s)
{
    /*
     * TODO:
     *  6. The worker/scheduler context is responsible for calling
     *     tcb_destroy(self) AFTER the switch completes.
     *
     * NOTE: Do NOT call tcb_destroy here. We are still running
     * on self's stack. Freeing it now is undefined behavior.
     */

    tcb_t *self = s->current_tcb;
    if (self == NULL) {
        return;
    }
    
    self->state = THREAD_FINISHED;
    if (self->joiner != NULL) {
        scheduler_enqueue(s, self->joiner);
    }

    tcb_t *next = scheduler_dequeue(s);
    if (next == NULL) {
        // TODO: switch to scheduler context here
        // for now this is a fatal condition
        next = NULL;
        fprintf(stderr, "scheduler_thread_exit: no next thread and no scheduler context\n");
        abort();
    }
    s->current_tcb = next;

    switch_context(&self->sp, next->sp); // TODO: why not tcb destroy after context switch?
}


void scheduler_join(scheduler_t *s, tcb_t *target)
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

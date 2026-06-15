#include "scheduler.h"
#include "scheduler_struct.h"
#include "tcb.h"
#include "context.h"
#include "spinlock.h"
#include "queue.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Default scheduler used by entities that don't have access to scheduler creation (like tcb.c)
static scheduler_t *default_scheduler = NULL;

/* ---------------------------------------------------------------
 * Lifecycle
 * --------------------------------------------------------------- */

void scheduler_init(void) {
    default_scheduler = scheduler_create();
} 

void scheduler_shutdown(void) {
    scheduler_destroy(default_scheduler);
    default_scheduler = NULL;
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

void scheduler_enqueue_s(scheduler_t *s, tcb_t *t)
{
    t->state = THREAD_READY;
    queue_push(&s->ready_queue, t);
}

void scheduler_enqueue(tcb_t *t) {
    scheduler_enqueue_s(default_scheduler, t);
}

tcb_t *scheduler_dequeue_s(scheduler_t *s)
{
    tcb_t *t = queue_pop(&s->ready_queue);
    if (t != NULL) {
        t->state = THREAD_RUNNING;
    }
    s->current_tcb = t;

    return t;
}

tcb_t *scheduler_dequeue(void) {
    return scheduler_dequeue_s(default_scheduler);
}

/* ---------------------------------------------------------------
 * Current-thread tracking
 * --------------------------------------------------------------- */

tcb_t *scheduler_current_s(scheduler_t *s)
{
    return s->current_tcb;
}

tcb_t *scheduler_current(void) {
    return scheduler_current_s(default_scheduler);
}

/* ---------------------------------------------------------------
 * Scheduling actions
 * --------------------------------------------------------------- */

void scheduler_yield_s(scheduler_t *s)
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
    scheduler_enqueue_s(s, prev);

    // TODO: what to switch to if next == NULL?
    tcb_t *next = scheduler_dequeue_s(s);
    s->current_tcb = next;

    switch_context(&prev->sp, next->sp);
}

void scheduler_yield(void) {
    scheduler_yield_s(default_scheduler);
}

void scheduler_thread_exit_s(scheduler_t *s)
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
        scheduler_enqueue_s(s, self->joiner);
    }

    tcb_t *next = scheduler_dequeue_s(s);
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

void scheduler_thread_exit(void) {
    scheduler_thread_exit_s(default_scheduler);
}

void scheduler_join_s(scheduler_t *s, tcb_t *target)
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

void scheduler_join(tcb_t *target) {
    scheduler_join_s(default_scheduler, target);
}

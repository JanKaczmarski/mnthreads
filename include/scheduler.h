#ifndef MNTHREAD_SCHEDULER_H
#define MNTHREAD_SCHEDULER_H

#include "tcb.h"
#include "queue.h"
#include "spinlock.h"

/*
 * scheduler.h -- Core scheduler state and operations.
 *
 * The scheduler owns the global ready queue and coordinates
 * how Workers pick up and run user threads.
 *
 * Step 3 (1:N): Single-threaded, no locking needed.
 * Step 4 (M:N): Multiple Workers, spinlock protects the queue.
 */

/* ---------------------------------------------------------------
 * Scheduler lifecycle
 * --------------------------------------------------------------- */

/*
 * Initialize the scheduler. Call once at startup before creating
 * any threads.
 */
void scheduler_init(void);

/*
 * Shut down the scheduler. Waits for all Workers to drain the
 * queue and exit. Call once at teardown.
 */
void scheduler_shutdown(void);

/* ---------------------------------------------------------------
 * Queue operations (lock-aware)
 * --------------------------------------------------------------- */

/*
 * Add a READY thread to the back of the global ready queue.
 * Acquires the spinlock internally in M:N mode.
 */
void scheduler_enqueue(tcb_t *t);

/*
 * Remove and return the next READY thread from the front of the
 * queue. Returns NULL if the queue is empty.
 * Acquires the spinlock internally in M:N mode.
 */
tcb_t *scheduler_dequeue(void);

/* ---------------------------------------------------------------
 * Current-thread tracking
 * --------------------------------------------------------------- */

/*
 * Get the TCB of the thread currently running on this Worker
 * (uses thread-local storage in M:N mode).
 */
tcb_t *scheduler_current(void);

/*
 * Set the current-thread pointer for this Worker.
 */
void scheduler_set_current(tcb_t *t);

/* ---------------------------------------------------------------
 * Scheduling actions (called by user threads)
 * --------------------------------------------------------------- */

/*
 * Voluntarily yield the CPU. Pushes the current thread back onto
 * the ready queue and switches to the Worker/scheduler context.
 *
 * Step 3: switches directly to the next ready thread.
 * Step 4: switches back to the Worker loop, which then picks
 *         the next thread from the queue.
 */
void scheduler_yield(void);

/*
 * Terminate the current thread. Marks it FINISHED, wakes its
 * joiner (if any), and switches away. The thread's resources
 * are freed by the scheduler/worker after the switch, not by
 * the thread itself (since it's still on its own stack).
 */
void scheduler_thread_exit(void);

/*
 * Block the calling thread until target finishes. If target is
 * already FINISHED, returns immediately. Otherwise, the caller
 * is moved to BLOCKED state and recorded as target->joiner.
 */
void scheduler_join(tcb_t *target);

#endif /* MNTHREAD_SCHEDULER_H */

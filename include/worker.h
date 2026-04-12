#ifndef MNTHREAD_WORKER_H
#define MNTHREAD_WORKER_H

#include <pthread.h>

/*
 * worker.h -- Kernel thread (pthread) worker pool.
 *
 * Each Worker is a real OS thread that runs an infinite loop:
 *   1. Dequeue a user thread from the ready queue
 *   2. switch_context into it
 *   3. When the user thread yields or exits, control returns here
 *   4. Handle post-switch cleanup (free finished threads, etc.)
 *   5. Go back to step 1
 *
 * Workers have their own saved stack pointer so that user threads
 * can switch_context back to the worker loop on yield/exit.
 *
 * Not needed for Step 3 (1:N cooperative model). Introduced in
 * Step 4 (M:N model).
 */

/* Maximum number of kernel worker threads. */
#define MAX_WORKERS 16

typedef struct {
    pthread_t   pthread;     /* The underlying OS thread               */
    int         id;          /* Worker index (0..num_workers-1)        */
    void       *sp;          /* Saved stack pointer for the worker's
                                own context (so user threads can
                                switch back to us)                     */
    int         running;     /* 1 if this worker should keep looping,
                                0 to signal shutdown                   */
} worker_t;

/*
 * Spawn num_workers kernel threads. Each enters worker_loop().
 * Call after scheduler_init().
 */
void workers_start(int num_workers);

/*
 * Signal all workers to stop and join their pthreads.
 * Call during scheduler_shutdown().
 */
void workers_stop(void);

/*
 * The main loop executed by each worker pthread.
 * Passed to pthread_create as the start routine.
 *
 * arg: pointer to this worker's worker_t struct.
 */
void *worker_loop(void *arg);

/*
 * Get the worker struct for the current kernel thread.
 * Uses thread-local storage. Returns NULL if called from
 * a non-worker thread (e.g., the main thread in Step 3).
 */
worker_t *worker_self(void);

#endif /* MNTHREAD_WORKER_H */

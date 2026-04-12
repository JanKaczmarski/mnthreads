#ifndef MNTHREAD_H
#define MNTHREAD_H

/*
 * mnthread.h -- Public API for the M:N threading library.
 *
 * This is the only header a user of the library needs to include.
 * It wraps the internal scheduler, TCB, and worker machinery
 * behind a clean interface.
 *
 * Usage:
 *   mnthread_init(4);                  // 4 kernel workers
 *   int id = mnthread_spawn(my_func);  // create a user thread
 *   mnthread_join(id);                 // wait for it
 *   mnthread_shutdown();               // tear down
 */

/*
 * Initialize the threading runtime with the given number of
 * kernel worker threads. Use num_workers=0 for cooperative
 * mode (Step 3: single OS thread, no pthreads).
 *
 * Must be called once before any other mnthread_* function.
 */
void mnthread_init(int num_workers);

/*
 * Spawn a new user-level thread that will execute func().
 *
 * Returns a thread ID (>= 0) on success, or -1 on failure.
 * The thread is placed in the ready queue immediately.
 */
int mnthread_spawn(void (*func)(void));

/*
 * Voluntarily yield the current thread's execution.
 * The thread is placed back in the ready queue and another
 * thread is scheduled.
 */
void mnthread_yield(void);

/*
 * Terminate the current thread. Called implicitly when a
 * thread's function returns (via the trampoline), but can
 * also be called explicitly.
 */
void mnthread_exit(void);

/*
 * Block the calling thread until the thread with the given
 * ID has finished. If the target thread has already finished,
 * returns immediately.
 *
 * Returns 0 on success, -1 if thread_id is invalid.
 */
int mnthread_join(int thread_id);

/*
 * Shut down the threading runtime. Signals all workers to
 * stop, joins their pthreads, and frees all remaining
 * resources.
 *
 * Must be called from the main thread after all user threads
 * have finished.
 */
void mnthread_shutdown(void);

#endif /* MNTHREAD_H */

#ifndef MNTHREAD_TCB_H
#define MNTHREAD_TCB_H

#include <stddef.h>

/*
 * tcb.h -- Thread Control Block.
 *
 * Every user-level thread is represented by one tcb_t.
 * The TCB owns the thread's stack memory and tracks its
 * execution state.
 */

/* Default stack size for user threads: 64 KB. */
#define MNTHREAD_STACK_SIZE (64 * 1024)

typedef enum {
    THREAD_READY,       /* In the ready queue, waiting to be scheduled   */
    THREAD_RUNNING,     /* Currently executing on a Worker               */
    THREAD_YIELDING,    /* In the process of yielding (transient state)  */
    THREAD_BLOCKED,     /* Waiting on a join or future synchronization   */
    THREAD_FINISHED     /* Function returned; awaiting cleanup           */
} thread_state_t;

typedef struct tcb {
    int             id;          /* Unique thread ID                      */

    void           *stack_base;  /* Base of mmap'd region (for munmap)    */
    size_t          stack_size;  /* Total size of the allocated stack     */

    void           *sp;          /* Saved stack pointer (read/written by
                                    switch_context)                       */

    thread_state_t  state;       /* Current lifecycle state               */

    void          (*entry)(void); /* The user function this thread runs   */

    struct tcb     *next;        /* Intrusive queue linkage               */

    struct tcb     *joiner;      /* Thread blocked in thread_join() on us.
                                    When we finish, re-enqueue the joiner.*/
} tcb_t;

/*
 * Allocate a new TCB and its stack. Set up the initial stack frame
 * so that the first switch_context into this thread will execute
 * the trampoline, which calls func().
 *
 * Returns a fully initialized TCB in THREAD_READY state, or NULL
 * on allocation failure.
 */
tcb_t *tcb_create(void (*func)(void));

/*
 * Free the TCB and its stack.
 *
 * IMPORTANT: Must NOT be called while the thread is still running
 * on its own stack. Switch to a different context first (e.g. the
 * scheduler/worker context), then call this.
 */
void tcb_destroy(tcb_t *t);

/*
 * The trampoline function. This is placed on the initial stack frame
 * as the "return address" for the first switch_context. It calls the
 * thread's entry function and then calls thread_exit() when the
 * entry function returns.
 *
 * You should not call this directly.
 */
void tcb_trampoline(void);

#endif /* MNTHREAD_TCB_H */

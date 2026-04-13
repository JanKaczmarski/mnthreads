#include "tcb.h"
#include "context.h"
#include "scheduler.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>

/* Monotonically increasing thread ID counter. */
static int next_id = 0;

/*
 * The trampoline is the first thing a new thread "returns" into
 * after its initial switch_context. It calls the thread's entry
 * function and handles cleanup when that function returns.
 */
void tcb_trampoline(void)
{
    /*
     * TODO:
     *  1. Get the currently running TCB (scheduler_current()).
     *  2. Call tcb->entry().
     *  3. When entry() returns, call scheduler_thread_exit().
     *     This ensures the thread doesn't RET into garbage.
     */
}

tcb_t *tcb_create(void (*func)(void))
{
    // NOTE: TCB is a structure that stores: 
    // - Thread context structure with relevenat register, when the context switch occures these
    // values are loaded to physical registers of CPU that allow for thread to resume.
    // - When Thread is not in Running state the Thread registers will be written to memory
    // and this mem pointer is stored in TCB
    // - TCB stores other important elements like ThreadId, Thread Memory, Kernel Stack, Parent ID
    // and other important metadata about Thread that allows it to be scheduled and descheduled.
    
    /*
     * TODO:
     *  1. Allocate a tcb_t (malloc is fine for the struct itself).
     *  2. Allocate the stack with mmap:
     *       - PROT_READ | PROT_WRITE
     *       - MAP_PRIVATE | MAP_ANONYMOUS
     *       - Optionally mprotect the bottom page as PROT_NONE
     *         (guard page to catch stack overflow).
     *  3. Compute the stack top:
     *       stack_top = (char *)stack_base + stack_size;
     *  4. Set up the initial stack frame at the top (growing down):
     *
     *       Slot (offset from top)    Value
     *       ─────────────────────     ─────────────────────
     *       -8   (return address)     address of tcb_trampoline
     *       -16  (saved RBP)          0
     *       -24  (saved RBX)          0
     *       -32  (saved R12)          0
     *       -40  (saved R13)          0
     *       -48  (saved R14)          0
     *       -56  (saved R15)          0
     *
     *     The stack pointer in the TCB should point to the R15
     *     slot (the lowest address), because switch_context will
     *     pop upward from there.
     *
     *  5. Fill in the rest of the TCB fields:
     *       - id = next_id++
     *       - state = THREAD_READY
     *       - entry = func
     *       - next = NULL
     *       - joiner = NULL
     *
     *  6. Return the TCB.
     */

    (void)func;
    return NULL;
}

void tcb_destroy(tcb_t *t)
{
    // NOTE: A mechanism used by OS to forcefully delete a Thread Control Block

    /*
     * TODO:
     *  1. munmap(t->stack_base, t->stack_size) to free the stack.
     *  2. free(t) to free the TCB struct itself.
     *
     * IMPORTANT: This must be called AFTER switching off this
     * thread's stack. Never free a stack you're still running on.
     */

    (void)t;
}

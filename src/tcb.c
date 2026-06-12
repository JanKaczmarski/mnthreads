#include "tcb.h"
#include "context.h"
#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
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
    tcb_t* current = scheduler_current();

    current->entry();

    scheduler_thread_exit();
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
     * Steps:
     *  1. Allocate a tcb_t (malloc is fine for the struct itself).
     *  2. Allocate the stack with mmap:
     *       - PROT_READ | PROT_WRITE
     *       - MAP_PRIVATE | MAP_ANONYMOUS
     *       - Optionally mprotect the bottom page as PROT_NONE
     *         (guard page to catch stack overflow).
     *  3. Compute the stack top:
     *       stack_top = (char *)stack_base + stack_size;
     *  4. Reserve a context_frame_t at the top of the stack
     *     (growing down). AArch64 layout, lowest address first
     *     (this matches the order that switch_context's ldp will
     *     pop registers off the stack):
     *
     *       Offset in frame   Register pair
     *       ───────────────   ──────────────
     *       +0,  +8           x29 (FP), x30 (LR)  ← lowest addr, popped first
     *       +16, +24          x27, x28
     *       +32, +40          x25, x26
     *       +48, +56          x23, x24
     *       +64, +72          x21, x22
     *       +80, +88          x19, x20            ← highest addr, popped last
     *
     *     Place tcb_trampoline's address in the x30 slot so the
     *     `ret` at the end of switch_context jumps into it.
     *
     *     tcb->sp must point to the start of the frame (the x29
     *     slot, lowest address) since `ldp` pops upward.
     *     The frame is 96 bytes (12 × 8), preserving the required
     *     16-byte SP alignment.
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

    tcb_t *tcb = (tcb_t *) malloc(sizeof(tcb_t));
    if (tcb == NULL) {
        return NULL;
    }

    // setup stack
    tcb->stack_size = MNTHREAD_STACK_SIZE;

    // full-descending stack in aarch64
    void *stack_base = mmap(NULL, MNTHREAD_STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (stack_base == MAP_FAILED) {
        perror("mmap allocation failed for new tcb");
        free(tcb);
        return NULL;
    }
    tcb->stack_base = stack_base;

    context_frame_t *frame = (context_frame_t *)((char *)stack_base + 
        MNTHREAD_STACK_SIZE -
        sizeof(context_frame_t));
    frame->x30 = (uint64_t)tcb_trampoline;

    tcb->sp = frame;

    // other fields setup
    tcb->state = THREAD_READY;
    tcb->id = next_id++;
    tcb->entry = func;
    tcb->next = NULL;
    tcb->joiner = NULL;

    return tcb;
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
    if (munmap(t->stack_base, t->stack_size) == -1) {
        perror("munmap failed on tcb");
    }

    free(t);
}

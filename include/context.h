#ifndef MNTHREAD_CONTEXT_H
#define MNTHREAD_CONTEXT_H

#include <stdint.h>

/*
 * context.h -- Low-level context switch primitive.
 *
 * The single most important function in the entire project.
 * Implemented in src/context_switch.S (not in C).
 *
 * Saves callee-saved registers (x19–x29, x30/LR) of the current
 * context onto its stack, stores SP into *old_sp, loads new_sp
 * into SP, restores the new context's callee-saved registers, and
 * executes RET (jumps to address restored into x30 from the new
 * stack).
 *
 * Parameters:
 *   old_sp  -- pointer to where the current SP should be saved
 *              (typically &current_tcb->sp)
 *   new_sp  -- the stack pointer to switch to
 *              (typically next_tcb->sp)
 */
void switch_context(void **old_sp, void *new_sp);

/*
 * context_frame is a struct that represents the layout used by context_switch.
 * Use this type to avoid errors related with manual stack allocation.
 *
 * Field order must mirror stp/ldp order in switch_context.
 */
typedef struct context_frame {
    uint64_t x29;
    uint64_t x30;
    uint64_t x27;
    uint64_t x28;
    uint64_t x25;
    uint64_t x26;
    uint64_t x23;
    uint64_t x24;
    uint64_t x21;
    uint64_t x22;
    uint64_t x19;
    uint64_t x20;
} context_frame_t;

#endif /* MNTHREAD_CONTEXT_H */

#ifndef MNTHREAD_CONTEXT_H
#define MNTHREAD_CONTEXT_H

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

#endif /* MNTHREAD_CONTEXT_H */

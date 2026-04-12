#ifndef MNTHREAD_CONTEXT_H
#define MNTHREAD_CONTEXT_H

/*
 * context.h -- Low-level context switch primitive.
 *
 * The single most important function in the entire project.
 * Implemented in src/context_switch.S (not in C).
 *
 * Saves callee-saved registers (RBX, RBP, R12-R15) of the current
 * context onto its stack, stores RSP into *old_sp, loads new_sp
 * into RSP, pops the new context's callee-saved registers, and
 * executes RET (which jumps to whatever address is on top of the
 * new stack).
 *
 * Parameters:
 *   old_sp  -- pointer to where the current RSP should be saved
 *              (typically &current_tcb->sp)
 *   new_sp  -- the stack pointer to switch to
 *              (typically next_tcb->sp)
 */
void switch_context(void **old_sp, void *new_sp);

#endif /* MNTHREAD_CONTEXT_H */

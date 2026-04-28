/*
 * context_switch.S -- Low-level context switch for AArch64 (AAPCS64 ABI).
 *
 * void switch_context(void **old_sp, void *new_sp);
 *
 * Function switches registers from old_stack_pointer values to new_stack_pointer values.
 */

.globl switch_context
switch_context:
    // save registers of current context - push to stack
    stp x19, x20, [sp, #-16]!
    stp x21, x22, [sp, #-16]!
    stp x23, x24, [sp, #-16]!
    stp x25, x26, [sp, #-16]!
    stp x27, x28, [sp, #-16]!
    stp x29, x30, [sp, #-16]!

    // store sp in x0
    str sp, [x1]

    // context switch - use
    mov sp, x1

    // restore registers - pop from stack
    ldp x29, x30, [sp], #16
    ldp x27, x28, [sp], #16
    ldp x25, x26, [sp], #16
    ldp x23, x24, [sp], #16
    ldp x21, x22, [sp], #16
    ldp x19, x20, [sp], #16

    // uses x30 register value for return
    ret


/*
 * context_switch.S -- Low-level context switch for x86_64 (System V ABI).
 *
 * void switch_context(void **old_sp, void *new_sp);
 *
 * On entry (System V calling convention):
 *   RDI = old_sp   (pointer to where we save current RSP)
 *   RSI = new_sp   (stack pointer to switch to)
 *
 * What this function does:
 *   1. Push all callee-saved registers onto the CURRENT stack
 *      (RBX, RBP, R12, R13, R14, R15)
 *   2. Save RSP into *old_sp
 *   3. Load new_sp into RSP
 *   4. Pop callee-saved registers from the NEW stack
 *   5. RET (pops return address from new stack and jumps to it)
 *
 * The matching initial stack layout (set up by tcb_create) must
 * have these same registers pushed in the same order, with the
 * thread's entry point (trampoline address) as the return address
 * on top.
 *
 * Stack layout after pushes (low address at top):
 *
 *   RSP --> [ R15    ]   <-- top of stack after saving
 *           [ R14    ]
 *           [ R13    ]
 *           [ R12    ]
 *           [ RBX    ]
 *           [ RBP    ]
 *           [ retaddr ]  <-- where RET will jump to
 */

#if defined(__APPLE__)
    /* macOS: symbols need a leading underscore */
    .globl _switch_context
    _switch_context:
#else
    /* Linux: no leading underscore */
    .globl switch_context
    switch_context:
#endif

    /* TODO: Implement this -- ~15 instructions total.
     *
     * Pseudocode:
     *
     *   push rbp
     *   push rbx
     *   push r12
     *   push r13
     *   push r14
     *   push r15
     *
     *   mov [rdi], rsp    // *old_sp = current RSP
     *   mov rsp, rsi      // RSP = new_sp
     *
     *   pop r15
     *   pop r14
     *   pop r13
     *   pop r12
     *   pop rbx
     *   pop rbp
     *
     *   ret                // jump to address on top of new stack
     */

    ret   /* placeholder -- remove when implementing */

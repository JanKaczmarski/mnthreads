# mnthreads — M:N User-Level Threading Library

A from-scratch M:N threading model in C for x86_64 (macOS/Linux). N user-level threads are multiplexed across M kernel threads (pthreads) using a hand-written assembly context switch.

## Project Structure

```
include/
  context.h       switch_context declaration
  spinlock.h      CAS-based spinlock
  queue.h         intrusive FIFO queue for TCBs
  tcb.h           Thread Control Block — stack, state, linkage
  scheduler.h     ready queue, yield, exit, join
  worker.h        pthread worker pool
  mnthread.h      public API (only header users need)

src/
  context_switch.S   x86_64 context switch (~15 instructions)
  spinlock.c         atomic_flag spinlock
  queue.c            linked-list queue operations
  tcb.c              TCB allocation, stack setup, trampoline
  scheduler.c        scheduling logic (1:N and M:N paths)
  worker.c           worker loop and pthread lifecycle
  mnthread.c         public API wiring

tests/
  test_context.c     Step 1 — raw ping-pong
  test_create.c      Step 2 — TCB + trampoline
  test_coop.c        Step 3 — cooperative 1:N
  test_mn.c          Step 4 — M:N with spinlocked queue
  test_join.c        Step 5 — thread join
```

## Build

```
make                 # build all tests
make test_context    # build + run Step 1
make test_create     # build + run Step 2
make test_coop       # build + run Step 3
make test_mn         # build + run Step 4
make test_join       # build + run Step 5
make clean
```

Compiler flags are set to `-g3 -O0 -mno-red-zone -Wall -Wextra`. Do not change `-O0` or `-mno-red-zone` until the project is completely finished — optimizations destroy hand-crafted stack frames, and the red zone will corrupt context switches.

ASan/UBSan lines are in the Makefile but commented out. They conflict with custom stacks; use `__attribute__((no_sanitize("address")))` on context-switch-adjacent code if you enable them.

## Implementation Plan

Do not proceed to the next step until the current one is completely stable. A subtle bug carried forward becomes undebuggable by Step 4.

### Step 1: Context Switch (the foundation)

Implement `switch_context` in `context_switch.S`. It saves callee-saved registers (RBP, RBX, R12–R15) onto the current stack, stores RSP into `*old_sp`, loads `new_sp` into RSP, pops the new context's registers, and RETs.

Implement `setup_stack` in `test_context.c`: lay out a fake stack frame (return address + 6 zeroed register slots) so that the first `switch_context` into it boots a C function.

**Pass criteria:** Two functions ping-pong 10,000 times with no crash. Spend as long as needed here — everything else depends on this being bulletproof.

**Key pitfalls:**
- 16-byte stack alignment at the point of CALL. Off-by-8 errors produce SIGSEGVs inside printf/malloc with no obvious cause.
- On macOS, symbols in `.S` files need a leading underscore (`_switch_context`). This is already handled in the template.

### Step 2: TCB and Stack Allocation

Implement `tcb_create` in `tcb.c`. Allocate the stack with `mmap` (not malloc) — use a `PROT_NONE` guard page at the bottom to catch overflow. Set up the initial stack frame with `tcb_trampoline` as the return address.

Implement the trampoline: it calls `scheduler_current()->entry()`, and when that returns, calls `scheduler_thread_exit()`. Without this, a returning thread RETs into garbage.

Also implement `spinlock.c` and `queue.c` — they are straightforward and needed by Step 3.

**Pass criteria:** Manually switching into a TCB runs the function and exits cleanly through the trampoline.

### Step 3: Cooperative 1:N Scheduling

Implement `scheduler_yield` (cooperative path only) and `scheduler_thread_exit` in `scheduler.c`. The ready queue is a FIFO; yield pushes the current TCB to the back, pops the next, and calls `switch_context`.

Use `mnthread_init(0)` — no workers, single OS thread.

Handle thread exit carefully: do NOT free a stack while you're still running on it. Switch to the scheduler/idle context first, then destroy the finished TCB.

**Pass criteria:** 5+ threads round-robin for 100,000 yield cycles with no crash.

### Step 4: M:N Architecture

Implement `worker.c` and the M:N code paths in `scheduler.c`. Each worker pthread runs a loop: lock queue, dequeue TCB, unlock, switch into it. On yield, the user thread switches back to the worker context (not directly to another user thread).

**The yield race condition** is the central challenge: between enqueueing a yielding TCB and completing the context switch, another worker can dequeue and run it. Solutions: hold the lock across enqueue + switch, or use a TCB state machine (RUNNING → YIELDING → READY) with atomics.

Worker idle behavior: when the queue is empty, `usleep` briefly rather than spinning at 100% CPU. A condition variable is better but adds complexity.

**Pass criteria:** 16 user threads on 4 workers, 10,000 yields each, no crash or deadlock. Run under Helgrind if on Linux (expect false positives on custom stacks).

### Step 5: Thread Join

Implement `scheduler_join` in `scheduler.c`. If the target is not finished, the caller sets `target->joiner = self`, marks itself BLOCKED (not READY), and switches away. When the target exits, `scheduler_thread_exit` re-enqueues the joiner.

Watch for the race where the target finishes between checking its state and setting the joiner pointer. Use the queue lock or an atomic state check.

**Pass criteria:** `main` joins all spawned threads and only exits after all complete. No premature process termination.

### Step 6: Preemption (optional stretch goal)

Use `setitimer(ITIMER_REAL, ...)` to fire SIGALRM every 10ms. The signal handler forces a yield.

This is significantly harder than Steps 1–5 combined. The core problem: if SIGALRM fires while holding the queue spinlock (or inside malloc/printf), switching away causes deadlock or heap corruption. Minimum mitigations: mask SIGALRM with `pthread_sigmask` inside critical sections, use `write()` not `printf` in signal handlers, never malloc from the handler.

A working cooperative M:N scheduler (Steps 1–5) is already a complete project. Only attempt this with time to spare.

## Extensions (low-latency / HFT focus)

These build directly on the core project and exercise the kind of reasoning trading firms (Jane Street, HRT, etc.) and core infrastructure teams care about.

### Extension A: Lock-Free Ready Queue

Replace the spinlock-protected queue in Step 4 with a lock-free Michael-Scott queue using CAS. This forces you to deal with the ABA problem, safe memory reclamation (hazard pointers or epoch-based), and the distinction between lock-free and wait-free. The yield race condition becomes harder — you can no longer just "hold the lock across the switch."

### Extension B: Context Switch Benchmarking

Measure your context switch latency in nanoseconds using `clock_gettime(CLOCK_MONOTONIC)` or `rdtsc`. Compare against:
- Raw `pthread` context switch (create two pthreads, ping-pong via a mutex+condvar)
- Go goroutine switch (`runtime.Gosched()` in a loop)

Being able to say "my switch is 45ns vs pthread's 800ns, here's why" demonstrates the right level of systems reasoning. Understand where the difference comes from: your switch saves 6 registers and swaps RSP; a pthread switch goes through the kernel, saves FP/SSE state, and may flush TLB entries.

### Extension C: Lock-Free SPSC Ring Buffer

A standalone follow-up project (a weekend once Steps 1–5 are done): implement a single-producer single-consumer ring buffer using only atomic loads and stores — no locks, no CAS. This is the most common low-latency primitive in trading systems (market data feeds, order book updates). The key insight is that SPSC needs only `memory_order_acquire` / `memory_order_release`, not `seq_cst`. Small enough to fit in a single `.c` file, deep enough to solidify your memory ordering understanding.

## Debugging

When you hit a segfault, go to GDB/LLDB immediately. Do not add printf statements to triangulate.

```
lldb ./test_context_bin
(lldb) run
(lldb) register read
(lldb) memory read -s8 -fx -c16 $rsp
```

## Prerequisites

- **Read before starting:** OSTEP (ostep.org), Part I (CPU scheduling) and Part II (concurrency).
- **Read alongside Step 1:** System V AMD64 ABI spec, section 3.2 (register table, stack alignment).
- **Read alongside Step 4:** Kerrisk's TLPI, chapters on pthreads and signals.

## Execution Strategy

- Hardcode everything first: fixed arrays of 64 TCBs, exactly 4 workers. Abstract later.
- Git commit after every working milestone. Name them clearly. You will need to revert.
- Stress test, not smoke test. 10 threads yielding once is a smoke test. 10,000 iterations with all threads active is a stress test. Concurrency bugs only surface under load.

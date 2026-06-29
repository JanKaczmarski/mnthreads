# AGENTS.md

## Commands
- Build all test binaries: `make`.
- Run focused tests by milestone: `make test_context`, `make test_create`, `make test_coop`, `make test_mn`, `make test_join`.
- Clean generated objects/binaries: `make clean`.
- Compiler flags intentionally use `-g3 -O0 -Wall -Wextra`; do not enable optimization while working on hand-built stacks/context frames.

## Current State
- This is an in-progress AArch64 C M:N threading library, not a passing project yet.
- On arm64 macOS, `make test_context` currently fails to link because `src/context_switch.S` exports `switch_context`, but clang references `_switch_context`.
- The scheduler refactor is incomplete: public declarations in `include/scheduler.h` take `scheduler_t *`, while some callers such as `src/tcb.c` still call `scheduler_current()` / `scheduler_thread_exit()` with no argument.
- `src/mnthread.c`, `src/worker.c`, parts of `src/scheduler.c`, and later tests still contain TODO implementations; do not assume later Make targets pass.

## Architecture Notes
- Public API is `include/mnthread.h`; internal pieces are scheduler, TCB, worker pool, ready queue, spinlock, and `src/context_switch.S`.
- `mnthread_init(0)` is cooperative 1:N mode with no pthread workers; `num_workers > 0` is intended for M:N mode.
- TCB stacks are allocated with `mmap`; never free/destroy a TCB while executing on its stack. Switch to a scheduler/worker context first.
- Initial AArch64 stack frames must match `switch_context` restore order and keep SP 16-byte aligned; x30 is the trampoline return target.
- M:N yield has a known race: a yielding TCB can be re-dequeued by another worker before the original worker switches off its stack. Resolve with lock coverage or a state machine before trusting Step 4.

## Verification Expectations
- Prefer milestone verification in order: `test_context` before TCB/scheduler work, then `test_create`, `test_coop`, `test_mn`, `test_join`.
- Sanitizer flags in the Makefile are commented out because ASan/UBSan can conflict with custom stacks; if enabling them, context-switch-adjacent code may need `no_sanitize("address")`.

/*
 * test_context.c -- Step 1: Context switch ping-pong.
 *
 * Two static stacks, two C functions, switch_context between them.
 * This test validates the assembly context switch in isolation,
 * before any TCB or scheduler code exists.
 */

#include "context.h"
#include "test_helpers.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STACK_SIZE 8192
#define ITERATIONS 10000

static char stack_A[STACK_SIZE] __attribute__((aligned(16)));
static char stack_B[STACK_SIZE] __attribute__((aligned(16)));

static void *sp_A    = NULL;
static void *sp_B    = NULL;
static void *sp_main = NULL;

static int counter_A = 0;
static int counter_B = 0;

static void func_A(void) {
  for (;;) {
    counter_A++;
    if (counter_A >= ITERATIONS) {
      // Ping-pong complete — return to main.
      switch_context(&sp_A, sp_main);
    }
    switch_context(&sp_A, sp_B);
  }
}

static void func_B(void) {
  for (;;) {
    counter_B++;
    switch_context(&sp_B, sp_A);
  }
}

// TODO: We could use context_frame_t in this testcase
/*
 * setup_stack creates a downward-growing stack for AArch64.
 * Lays out 12 zero'd 8-byte slots for x19-x30, placing func
 * in the x30 slot so that the first switch_context RET jumps into it.
 *
 *   sp -->  [ 0           ]   x29 (FP)
 *           [ func address]   x30 (LR / return address for RET)
 *           [ 0           ]   x27
 *           [ 0           ]   x28
 *            ...
 *           [ 0           ]   x19
 *           [ 0           ]   x20
 */
static void *setup_stack(char *stack_mem, size_t size, void (*func)(void)) {
  uintptr_t *sp = (uintptr_t *)(stack_mem + size);

  // 16-byte alignment
  sp = (uintptr_t *)((uintptr_t)sp & ~0xF);

  // Reserve 12 slots for callee-saved registers x19-x30
  sp -= 12;

  for (int i = 0; i < 12; i++) {
    sp[i] = 0;
  }

  // x30 (LR) slot is index 1: layout is [x29, x30, x27, x28, ...]
  sp[1] = (uintptr_t)func;

  return sp;
}

int test_context_switch_pingpong(void) {
  // Reset globals so the test is repeatable.
  counter_A = 0;
  counter_B = 0;
  sp_A = sp_B = sp_main = NULL;

  sp_A = setup_stack(stack_A, STACK_SIZE, func_A);
  sp_B = setup_stack(stack_B, STACK_SIZE, func_B);

  TEST_ASSERT(sp_A != NULL, "setup_stack for A returned NULL");
  TEST_ASSERT(sp_B != NULL, "setup_stack for B returned NULL");

  // Jump into func_A — returns here when func_A switches back to sp_main.
  switch_context(&sp_main, sp_A);

  TEST_ASSERT(counter_A == ITERATIONS,
      "func_A should have run exactly ITERATIONS times");
  TEST_ASSERT(counter_B == ITERATIONS - 1,
      "func_B should have run exactly ITERATIONS-1 times");

  return 1;
}

int main(void) {
  char *log_pref = "[main]";

  printf("%s: Starting test for `test_context_switch_pingpong`\n", log_pref);
  if (test_context_switch_pingpong() == 0) {
    printf("%s: FAILED test_context_switch_pingpong\n", log_pref);
    return 1;
  }

  printf("%s: all tests passed\n", log_pref);
  return 0;
}

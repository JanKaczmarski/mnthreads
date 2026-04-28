/*
 * test_context.c -- Step 1: Context switch ping-pong.
 *
 * Two static stacks, two C functions, switch_context between them.
 * This test validates the assembly context switch in isolation,
 * before any TCB or scheduler code exists.
 */

#include "../include/context.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 8192
#define ITERATIONS 10000

static char stack_A[STACK_SIZE] __attribute__((aligned(16)));
static char stack_B[STACK_SIZE] __attribute__((aligned(16)));

/* Each context saves/restores its RSP here. */
static void *sp_A = NULL;
static void *sp_B = NULL;
static void *sp_main = NULL;

static int counter_A = 0;
static int counter_B = 0;

static void func_A(void) {
  for (;;) {
    counter_A++;
    if (counter_A >= ITERATIONS) {
      /* Switch back to main to exit cleanly. */
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

/*
 * Helper: set up a stack so that switch_context into it will
 * "return" into func. Mimics the layout expected by the .S file:
 *
 *   top of stack -->  [ func address ]   (return address for RET)
 *                     [ 0            ]   (saved RBP)
 *                     [ 0            ]   (saved RBX)
 *                     [ 0            ]   (saved R12)
 *                     [ 0            ]   (saved R13)
 *                     [ 0            ]   (saved R14)
 *   sp ------------>  [ 0            ]   (saved R15)
 */
static void *setup_stack(char *stack_mem, size_t size, void (*func)(void)) {
  /*
   * TODO: Compute the top of the stack, ensure 16-byte alignment,
   * write the initial register slots and return address, and
   * return the resulting stack pointer.
   */
  (void)stack_mem;
  (void)size;
  (void)func;
  return NULL;
}

int main(void) {
  printf("=== Step 1: Context switch ping-pong ===\n");

  sp_A = setup_stack(stack_A, STACK_SIZE, func_A);
  sp_B = setup_stack(stack_B, STACK_SIZE, func_B);

  if (!sp_A || !sp_B) {
    fprintf(stderr, "setup_stack not yet implemented\n");
    return 1;
  }

  /* Jump into func_A. When it's done, it switches back here. */
  switch_context(&sp_main, sp_A);

  printf("func_A ran %d times\n", counter_A);
  printf("func_B ran %d times\n", counter_B);

  if (counter_A >= ITERATIONS) {
    printf("PASS: %d iterations completed.\n", ITERATIONS);
  } else {
    printf("FAIL: only completed %d iterations.\n", counter_A);
    return 1;
  }

  return 0;
}

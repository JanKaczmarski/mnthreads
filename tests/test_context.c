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

static void *sp_A = NULL;
static void *sp_B = NULL;
static void *sp_main = NULL;

static int counter_A = 0;
static int counter_B = 0;

static void func_A(void) {
  for (;;) {
    counter_A++;
    if (counter_A >= ITERATIONS) {
      // finish ping pong and go back to main
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
 *  setup stack creates a downwards stack for aarch64 given the stack ptr and size available
 *  it fills space for 12 8byte registers with 0 and on x30 (top of the stack) it plaes *func
 *  which is a callback function used by RET instruction.
 *
 *   top of stack, sp -->  [ 0        ]   (x29)
 *                     [ func addres  ]   (return addres for RET, x30)
 *                     [ 0            ]   (saved x27)
 *                     [ 0            ]
 *                      .
 *                      .
 *                     [ 0            ]   (saved x19)
 *                     [ 0            ]   (saved x20)
 */
static void *setup_stack(char *stack_mem, size_t size, void (*func)(void)) {
  uintptr_t *sp = (uintptr_t *)(stack_mem + size);

  // ensure 16-bit allignment
  sp = (uintptr_t *)((uintptr_t)sp & ~0xF);

  // make space for 12 registers x19-x30 - uintptr_t is 32bit for our arch (register size)
  sp -= 12;

  // zero every "register" value in stack
  for (int i = 0; i < 12; i++) {
    sp[i] = 0;
  }

  // x30 register has RET function. Stack: x29 - x30 - x27 - x28 - ...
  sp[1] = (uintptr_t)func;

  // sp points to x30 at top of the stack
  return sp;
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

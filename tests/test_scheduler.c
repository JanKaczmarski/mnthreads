#include "scheduler.h"
#include "scheduler_internal.h"
#include "scheduler_struct.h"
#include "tcb.h"
#include "test_helpers.h"

#include <stdint.h>
#include <stdio.h>

int test_scheduler_create(void) {
  scheduler_t *s = scheduler_create();

  TEST_ASSERT(s != NULL, "scheduler_create returned NULL");
  TEST_ASSERT(s->current_tcb == NULL, "current_tcb should be NULL on init");
  TEST_ASSERT(s->num_workers_active == 0, "num_workers_active should be 0 on init");

  scheduler_destroy(s);
  return 1;
}

int test_scheduler_enqueue(void) {
  scheduler_t *s = scheduler_create();
  tcb_t *t = tcb_create(NULL);

  TEST_ASSERT(s != NULL, "scheduler_create returned NULL");

  scheduler_enqueue(s, t);

  TEST_ASSERT(scheduler_queue_size(s) == 1, "queue size should be 1 after enqueue");

  scheduler_destroy(s);
  return 1;
}

int test_scheduler_dequeue(void) {
  scheduler_t *s = scheduler_create();
  tcb_t *t = tcb_create(NULL);

  TEST_ASSERT(s != NULL, "scheduler_create returned NULL");

  // dequeue on empty queue
  tcb_t *got_tcb = scheduler_dequeue(s);
  TEST_ASSERT(got_tcb == NULL, "dequeue on empty queue should return NULL");

  // dequeue on non-empty queue
  scheduler_enqueue(s, t);

  got_tcb = scheduler_dequeue(s);
  TEST_ASSERT(got_tcb == t, "dequeue should return the enqueued tcb");
  TEST_ASSERT(s->current_tcb == t, "current_tcb should be set to dequeued tcb");

  tcb_destroy(t);
  scheduler_destroy(s);
  return 1;
}

// Global allowing to return from `switch_context` in child function to test
// the stack pointer which "yielded". Used for test_scheduler_yield.
static scheduler_t *t_sched_yield = NULL;

static void yield_thread(void) {
  scheduler_yield(t_sched_yield);
}

int test_scheduler_yield(void) {
  scheduler_t *s = scheduler_create();
  scheduler_set_default(s); // required — tcb_trampoline calls scheduler_default()
  t_sched_yield = s;

  tcb_t main_stub = {0};
  s->current_tcb = &main_stub;
  tcb_t *t = tcb_create(yield_thread);
  scheduler_enqueue(s, t);

  scheduler_yield(s);

  // After full yield round-trip:
  //   1st yield: enqueues main_stub, dequeues t, switches to yield_thread
  //   yield_thread: enqueues t, dequeues main_stub, switches back here
  // Result: current_tcb = &main_stub, queue = [t]
  TEST_ASSERT(scheduler_queue_size(s) == 1, "queue size should be 1 after yield round-trip");
  TEST_ASSERT(s->ready_queue.head == t, "queue head should be t after yield round-trip");

  t_sched_yield = NULL;
  scheduler_reset_default();
  scheduler_destroy(s);
  return 1;
}

static void exit_thread(void) {
  // just returns — trampoline calls scheduler_thread_exit
}

int test_scheduler_thread_exit(void) {
  scheduler_t *s = scheduler_create();
  scheduler_set_default(s);

  // --- With current_tcb not being set ---
  scheduler_thread_exit(s);

  TEST_ASSERT(s->current_tcb == NULL, "current_tcb shouldn't change with scheduler_thread_exit on empty scheduler");
  TEST_ASSERT(s->ready_queue.size == 0, "ready_queue should be empty on scheduler_thread_exit on empty scheduler");

  // --- Thread runs to completion via scheduler_run ---
  tcb_t *t = tcb_create(exit_thread);
  scheduler_enqueue(s, t);

  scheduler_run(s);

  TEST_ASSERT(t->state == THREAD_FINISHED, "thread should be THREAD_FINISHED after exit");
  TEST_ASSERT(s->current_tcb == NULL, "current_tcb should be NULL after all threads exit");
  TEST_ASSERT(s->ready_queue.size == 0, "ready_queue should be empty after all threads exit");

  tcb_destroy(t);
  scheduler_reset_default();
  scheduler_destroy(s);
  return 1;
}

int main(void) {
  char *log_pref = "[main]";

  printf("%s: Starting test for `test_scheduler_create`\n", log_pref);
  if (test_scheduler_create() == 0) {
    printf("%s: FAILED test_scheduler_create\n", log_pref);
    return 1;
  }

  printf("%s: Starting test for `test_scheduler_enqueue`\n", log_pref);
  if (test_scheduler_enqueue() == 0) {
    printf("%s: FAILED test_scheduler_enqueue\n", log_pref);
    return 1;
  }

  printf("%s: Starting test for `test_scheduler_dequeue`\n", log_pref);
  if (test_scheduler_dequeue() == 0) {
    printf("%s: FAILED test_scheduler_dequeue\n", log_pref);
    return 1;
  }

  printf("%s: Starting test for `test_scheduler_yield`\n", log_pref);
  if (test_scheduler_yield() == 0) {
    printf("%s: FAILED test_scheduler_yield\n", log_pref);
    return 1;
  }

  printf("%s: Starting test for `test_scheduler_thread_exit`\n", log_pref);
  if (test_scheduler_thread_exit() == 0) {
    printf("%s: FAILED test_scheduler_thread_exit\n", log_pref);
    return 1;
  }

  printf("%s: all tests passed\n", log_pref);
  return 0;
}

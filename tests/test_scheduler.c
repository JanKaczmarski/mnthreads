#include "scheduler.h"
#include "scheduler_internal.h"
#include "scheduler_struct.h"
#include "tcb.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// scheduler_create
int test_scheduler_create(void) {
  char *test_func_name = "test_scheduler_create";

  scheduler_t *s = scheduler_create();

  if (s == NULL) {
    fprintf(stderr, "%s: scheduler create cannot be NULL\n", test_func_name);
    return 0;
  }

  if (s->current_tcb != NULL) {
    // TODO: How to give here a verbose error?
    fprintf(stderr, "%s: current_tcb should be equal to NULL, %lu\n", test_func_name, (uintptr_t) s->current_tcb);
    scheduler_destroy(s);
    return 0;
  }

  if (s->num_workers_active < 0) {
    fprintf(stderr, "%s: num_workers_active cannot be negative, %d\n", test_func_name, s->num_workers_active);
    scheduler_destroy(s);
    return 0;
  }

  scheduler_destroy(s);
  return 1;
}

// scheduler enqueue
// TODO: Maybe we can use a shared scheduler here to avoid malloc and free
int test_scheduler_enqueue(void) {
  char *test_func_name = "test_scheduler_enqueue";

  scheduler_t *s = scheduler_create();
  tcb_t *t = tcb_create(NULL);

  if (s == NULL) {
    fprintf(stderr, "%s: scheduler_create shouldn't create a NULL scheduler\n", test_func_name);
    return 0;
  }

  scheduler_enqueue(s, t);

  int got_queue_size = scheduler_queue_size(s);
  int want_queue_size = 1;

  if (got_queue_size != want_queue_size) {
    fprintf(stderr, "%s: scheduler_enqueue_s should add exactly %d tcb to queue, but the length of queue is `%d`\n", test_func_name, want_queue_size, got_queue_size);
    scheduler_destroy(s);
    return 0;
  }

  scheduler_destroy(s);
  return 1;
}

int test_scheduler_dequeue(void) {
  char *test_func_name = "test_scheduler_dequeue";

  scheduler_t *s = scheduler_create();
  tcb_t *t = tcb_create(NULL);

  if (s == NULL) {
    fprintf(stderr, "%s: scheduler_create shouldn't create a NULL scheduler\n", test_func_name);
    tcb_destroy(t);
    return 0;
  }

  // dequeue on empty queue
  tcb_t *got_tcb = scheduler_dequeue(s);
  if (got_tcb != NULL) {
    fprintf(stderr, "%s: scheduler_dequeue on empty scheduler should return NULL tcb\n", test_func_name);
    scheduler_destroy(s);
    return 0;
  }

  // dequeue on non-empty queue
  scheduler_enqueue(s, t); // assuming enqueue works properly

  got_tcb = scheduler_dequeue(s);
  if (got_tcb != t) {
    fprintf(stderr, "%s: expected address `%lu` for enqueued tcb, but got %lu\n", test_func_name, (uintptr_t)t, (uintptr_t)got_tcb);
    scheduler_destroy(s);
    return 0;
  }

  if (s->current_tcb != t) {
    fprintf(stderr, "%s: current_tcb on scheduler should be set to dequeued tcb. got `%lu`, want `%lu`\n", test_func_name, (uintptr_t)s->current_tcb, (uintptr_t)t);
    scheduler_destroy(s);
    return 0;
  }

  scheduler_destroy(s);

  return 1;
}

// Global allowing to return from `switch_context` in child funciton to test Stack Pointer
// which "yielded". Used for scheduler_yield_s tests
static scheduler_t *t_sched_yield = NULL;

static void yield_thread(void) {
  scheduler_yield(t_sched_yield);
}

int test_scheduler_yield(void) {
  char *test_func_name = "test_scheduler_yield";

  scheduler_t *s = scheduler_create();
  t_sched_yield = s;

  tcb_t main = {0};
  s->current_tcb = &main;
  tcb_t *t = tcb_create(yield_thread);
  
  scheduler_enqueue(s, t);

  scheduler_yield(s);
  int got_queue_size = scheduler_queue_size(s);
  int want_queue_size = 1;
  if (got_queue_size != want_queue_size) {
    fprintf(stderr, "%s: queue size after scheduler yield is incorrect. got `%d` want `%d`\n", test_func_name, got_queue_size, want_queue_size);
    scheduler_destroy(s);
    return 0;
  }

  tcb_t *got_queue_head = s->ready_queue.head;
  if (got_queue_head != t) { // scheduler_yield was called 2 times (1 time via yield_thread entrypoint)
    fprintf(stderr, "%s: queue head incorrect. got `%lu` want `%lu`", test_func_name, (uintptr_t)got_queue_head, (uintptr_t)t);
    scheduler_destroy(s);
    return 0;
  }

  t_sched_yield = NULL;
  scheduler_destroy(s);

  return 1;
}

// TODO: Add test case for scheduler_thread_exit_s
int test_scheduler_thread_exit(void) {
  fprintf(stderr, "test_scheduler_thread_exit: UNIMPLEMENTED\n");
  return 1;
}


int main(void) {
  char *log_pref = "[main]";

  printf("%s: Starting test for `test_scheduler_create`\n", log_pref);
  if (test_scheduler_create() == 0) {
    printf("%s: Test for `test_scheduler_create` failed\n", log_pref);
    return 0;
  }

  printf("%s: Starting test for `test_scheduler_enqueue`\n", log_pref);
  if (test_scheduler_enqueue() == 0) {
    printf("%s: Test for `test_scheduler_enqueue` failed\n", log_pref);
    return 0;
  }

  printf("%s: Starting test for `test_scheduler_dequeue`\n", log_pref);
  if (test_scheduler_dequeue() == 0) {
    printf("%s: Test for `test_scheduler_dequeue` failed\n", log_pref);
    return 0;
  }

  printf("%s: Starting test for `test_scheduler_yield`\n", log_pref);
  if (test_scheduler_yield() == 0) {
    printf("%s: Test for `test_scheduler_yield` failed\n", log_pref);
    return 0;
  }

  printf("%s: Starting test for `test_scheduler_thread_exit`\n", log_pref);
  if (test_scheduler_thread_exit() == 0) {
    printf("%s: Test for `test_scheduler_thread_exit` failed\n", log_pref);
    return 0;
  }
}



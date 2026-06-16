#ifndef MNTHREAD_SCHEDULER_INTERNAL_H
#define MNTHREAD_SCHEDULER_INTERNAL_H

#include "queue.h"
#include "scheduler.h"
#include "scheduler_struct.h" // NOLINT: private header used to reference struct
#include <stdbool.h>

static inline bool scheduler_queue_empty(scheduler_t *s) {
  return queue_empty(&s->ready_queue);
}

static inline int scheduler_queue_size(scheduler_t *s) {
  return queue_size(&s->ready_queue);
}

static inline tcb_t *scheduler_get_current(scheduler_t *s) {
  return s->current_tcb;
}

/*
 * Resets the package-wide default scheduler to NULL.
 * FOR TESTING ONLY — allows tests to set a fresh default per test case
 * without triggering the double-init assert in scheduler_set_default().
 */
void scheduler_reset_default(void);

#endif

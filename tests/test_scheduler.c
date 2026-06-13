#include "scheduler.h"

// scheduler_init

int test_scheduler_init(void) {
  scheduler_init();

  if (scheduler_current() != NULL) {
    return 0;
  }

  return 1;
}

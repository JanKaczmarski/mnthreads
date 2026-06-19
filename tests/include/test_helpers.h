#ifndef MNTHREAD_TEST_HELPERS_H
#define MNTHREAD_TEST_HELPERS_H

#include <stdio.h>

/*
 * TEST_ASSERT(cond, msg)
 *
 * Checks that `cond` is true. If not, prints the file, line number,
 * and a custom message to stderr, then returns 0 (failure) from the
 * enclosing test function.
 *
 * Usage:
 *   TEST_ASSERT(s != NULL, "scheduler_create returned NULL");
 *   TEST_ASSERT(s->current_tcb == NULL, "current_tcb should be NULL on init");
 */
#define TEST_ASSERT(cond, msg)                                          \
    do {                                                                \
        if (!(cond)) {                                                  \
            fprintf(stderr, "%s:%d: assertion failed: %s\n",           \
                    __FILE__, __LINE__, (msg));                         \
            return 0;                                                   \
        }                                                               \
    } while (0)

#endif /* MNTHREAD_TEST_HELPERS_H */

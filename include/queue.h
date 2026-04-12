#ifndef MNTHREAD_QUEUE_H
#define MNTHREAD_QUEUE_H

#include <stdbool.h>

/*
 * queue.h -- Intrusive FIFO queue for TCBs.
 *
 * "Intrusive" means the queue linkage (next pointer) lives inside
 * the TCB struct itself, so enqueue/dequeue never allocate memory.
 *
 * This queue is NOT thread-safe on its own. In the M:N model,
 * callers must hold the scheduler's spinlock before calling
 * queue_push / queue_pop.
 */

/* Forward declaration -- full definition is in tcb.h */
typedef struct tcb tcb_t;

typedef struct {
    tcb_t *head;
    tcb_t *tail;
    int    size;
} thread_queue_t;

/* Initialize an empty queue. */
void queue_init(thread_queue_t *q);

/* Push a TCB to the back of the queue. */
void queue_push(thread_queue_t *q, tcb_t *t);

/* Pop a TCB from the front. Returns NULL if empty. */
tcb_t *queue_pop(thread_queue_t *q);

/* Peek at the front without removing. Returns NULL if empty. */
tcb_t *queue_peek(const thread_queue_t *q);

/* True if the queue has no elements. */
bool queue_empty(const thread_queue_t *q);

/* Number of elements currently in the queue. */
int queue_size(const thread_queue_t *q);

#endif /* MNTHREAD_QUEUE_H */

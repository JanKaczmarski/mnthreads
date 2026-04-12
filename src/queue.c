#include "queue.h"
#include "tcb.h"
#include <stddef.h>

void queue_init(thread_queue_t *q)
{
    /* TODO: Set head, tail to NULL, size to 0. */
    (void)q;
}

void queue_push(thread_queue_t *q, tcb_t *t)
{
    /*
     * TODO: Append t to the tail of the list.
     * Set t->next = NULL.
     * If the queue was empty, head = tail = t.
     * Otherwise, tail->next = t; tail = t.
     * Increment size.
     */
    (void)q;
    (void)t;
}

tcb_t *queue_pop(thread_queue_t *q)
{
    /*
     * TODO: Remove and return the head.
     * If empty, return NULL.
     * If head == tail, set both to NULL.
     * Decrement size.
     */
    (void)q;
    return NULL;
}

tcb_t *queue_peek(const thread_queue_t *q)
{
    (void)q;
    return NULL;
}

bool queue_empty(const thread_queue_t *q)
{
    (void)q;
    return true;
}

int queue_size(const thread_queue_t *q)
{
    (void)q;
    return 0;
}

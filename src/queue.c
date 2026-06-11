#include "queue.h"
#include "tcb.h"
#include <stddef.h>
#include <time.h>

void queue_init(thread_queue_t *q)
{
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void queue_push(thread_queue_t *q, tcb_t *t)
{
    // last element in thread queue
    t->next = NULL;

    if (q->head == NULL) {
        q->head = q->tail = t;
    } else {
        q->tail->next = t;
        q->tail = t;
    }

    q->size++;
}

tcb_t *queue_pop(thread_queue_t *q)
{
    if (q->head == NULL) {
        return NULL;
    }

    tcb_t* t = q->head;

    q->head = t->next;

    if (q->head == NULL) {
        q->tail = NULL;
    }

    t->next = NULL;
    q->size--;

    return t;
}

tcb_t *queue_peek(const thread_queue_t *q)
{
    if (q->head == NULL) {
        return NULL;
    }

    return q->head;
}

bool queue_empty(const thread_queue_t *q)
{
    return q->head == NULL;
}

int queue_size(const thread_queue_t *q)
{
    return q->size;
}

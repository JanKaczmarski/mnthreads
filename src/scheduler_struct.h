#ifndef MNTHREAD_SCHEDULER_STRUCT_H
#define MNTHREAD_SCHEDULER_STRUCT_H

#include "queue.h"
#include "spinlock.h"
#include "tcb.h"

struct scheduler {
    thread_queue_t ready_queue;
    spinlock_t queue_lock;
    tcb_t *current_tcb;
    int num_workers_active;
};

#endif

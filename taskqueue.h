#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include <pthread.h>

typedef struct worker{
    pthread_t thread;
    int finish;
    struct taskqueue *taskq_ptr;
    struct worker *prev, *next;
}worker;

typedef struct job{
    void *data_ptr;
    struct job *prev, *next;
}job;

typedef struct taskqueue{
    struct worker *workers_ptr;
    struct job *jobs_ptr;
    void (*deal_func)(struct job *job_ptr);
    pthread_mutex_t task_mutex;
    pthread_cond_t task_cond;
}taskqueue;

int taskqueue_init(taskqueue *taskq_ptr, int workers_num);
void taskqueue_close(taskqueue *taskq_ptr);
void taskqueue_add_job(taskqueue *taskq_ptr, job *job_ptr);

#endif

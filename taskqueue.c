#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"taskqueue.h"


#define QUEUE_ADD(item, head) {\
    item->prev = NULL;\
    item->next = head;\
    head = item;\
}

#define QUEUE_REMOVE(item, head) {\
    if(item->prev != NULL) item->prev->next = item->next;\
    if(item->next != NULL) item->next->prev = item->prev;\
    if(head == item) head = item->next;\
    item->prev = item->next = NULL;\
}

static void *worker_func(void *ptr)
{
    worker *worker_ptr = (worker*) ptr;
    job *job_ptr;
    while(1){
        pthread_mutex_lock(&worker_ptr->taskq_ptr->task_mutex);
        while(worker_ptr->taskq_ptr->jobs_ptr == NULL){
            pthread_cond_wait(&worker_ptr->taskq_ptr->task_cond, 
                   &worker_ptr->taskq_ptr->task_mutex); 
        }
        job_ptr = worker_ptr->taskq_ptr->jobs_ptr;
        if(job_ptr != NULL){
            QUEUE_REMOVE(job_ptr, worker_ptr->taskq_ptr->jobs_ptr);
        }
        pthread_mutex_unlock(&worker_ptr->taskq_ptr->task_mutex);

        if(worker_ptr->finish) break;

        if(job_ptr == NULL) continue;

        worker_ptr->taskq_ptr->deal_func(job_ptr);
    }
    free(worker_ptr);
    pthread_exit(NULL);
}

int taskqueue_init(taskqueue *taskq_ptr, int workers_num)
{
    int i;
    worker *worker_ptr;

    memset(taskq_ptr, 0, sizeof(*taskq_ptr));

    for(i = 0; i < workers_num; i++){
        if((worker_ptr = malloc(sizeof(struct worker))) == NULL){
            perror("Failed to allocate mem to worker.");
            return 1;
        }
        memset(worker_ptr, 0, sizeof(*worker_ptr));
        worker_ptr->taskq_ptr = taskq_ptr;
        if(pthread_create(&worker_ptr->thread, NULL, worker_func, 
                    (void*)worker_ptr))
        {
            perror("Failed to create worker thread."); 
            free(worker_ptr);
            return 1;
        }
        QUEUE_ADD(worker_ptr, worker_ptr->taskq_ptr->workers_ptr);
    }
}

void taskqueue_close(taskqueue *taskq_ptr)
{
    worker *worker_ptr = NULL;
    for(worker_ptr = taskq_ptr->workers_ptr; worker_ptr != NULL;
            worker_ptr = worker_ptr->next){
        worker_ptr->finish= 1;
    }

    pthread_mutex_lock(&taskq_ptr->task_mutex);
    taskq_ptr->workers_ptr = NULL;
    taskq_ptr->jobs_ptr = NULL;
    pthread_cond_broadcast(&taskq_ptr->task_cond);
    pthread_mutex_unlock(&taskq_ptr->task_mutex);
}

void taskqueue_add_job(taskqueue *taskq_ptr, job *job_ptr)
{
    pthread_mutex_lock(&taskq_ptr->task_mutex);
    QUEUE_ADD(job_ptr, taskq_ptr->jobs_ptr);
    pthread_cond_signal(&taskq_ptr->task_cond);
    pthread_mutex_unlock(&taskq_ptr->task_mutex);
}

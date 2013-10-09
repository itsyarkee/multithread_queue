#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"taskqueue.h"

#define MAXLEN 1000
#define WORKER_NUM 16
#define JOB_NUM 200

static taskqueue taskq;

void todo(struct job *job_ptr)
{
    char *data_ptr = (char*)job_ptr->data_ptr;
    printf("%s\n", data_ptr);
    free(job_ptr->data_ptr);
    free(job_ptr);
}

int main()
{
    int i;
    job *job_ptr;
    char buffer[MAXLEN], *tmp[JOB_NUM];
    taskqueue_init(&taskq, WORKER_NUM);
    taskq.deal_func = todo;
    for(i = 0; i < JOB_NUM; i ++){
        if((job_ptr = malloc(sizeof(struct job))) == NULL){
            perror("Failed to allocate memory for job");
        }else{
            snprintf(buffer, MAXLEN, "The job num is %d", i);
            tmp[i] = strdup(buffer);
            job_ptr->data_ptr = tmp[i];
            taskqueue_add_job(&taskq, job_ptr);
        }
    }
    while(taskq.jobs_ptr != NULL);
    taskqueue_close(&taskq);
    return 0;
}

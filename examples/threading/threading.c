#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    //DEBUG_LOG("sleep %d",thread_func_args->wait_to_obtain_ms);
    sleep(thread_func_args->wait_to_obtain_ms/1000);
    //DEBUG_LOG("sleep over, waiting for mutex");
    int rc=pthread_mutex_lock(thread_func_args->mutex);
    if (rc!=0){
        printf("error %d: mutex lock failed\n",rc);
        thread_func_args->thread_complete_success=false;
        return false;
    }
    //DEBUG_LOG("obtained mutex, sleeping");
    sleep(thread_func_args->wait_to_release_ms/1000);
    //DEBUG_LOG("sleep over, unlocking mutex");

    rc=pthread_mutex_unlock(thread_func_args->mutex);
    if (rc!=0){
        printf("error %d: mutex unlock failed\n",rc);
        thread_func_args->thread_complete_success=false;
        return false;
    }
    //DEBUG_LOG("unlocked mutex, returning");

    thread_func_args->thread_complete_success=true;
    return (void*)thread_func_args;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data* thread_param=malloc(sizeof *thread_param);
    *thread_param=(struct thread_data){wait_to_obtain_ms,wait_to_release_ms,false,mutex};
    /*int rc=pthread_mutex_init(&thread_param->mutex,NULL);
    if (rc!=0){
        printf("error %d: mutex init failed",rc);
        return false;
    }*/
    int rc=pthread_create(thread,NULL,threadfunc,(void *)thread_param);
    if (rc!=0){
        printf("error %d: mutex init failed",rc);
        return false;
    }
    return true;
}


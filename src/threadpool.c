#include "threadpool.h"
#include "digest.h"
#include "cache.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <string.h>
#include "errExit.h"
#include "common.h"

static pending_request_t queue[MAX_QUEUE];
static long queue_count = 0;

static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

// takes the request with the smallest file size from the queue 
static pending_request_t dequeue_smallest(void) {

    int min_idx = 0;
    for (int i = 1; i < queue_count; i++) {
        if (queue[i].file_size < queue[min_idx].file_size) {
            min_idx = i;
        }
    }
    pending_request_t chosen = queue[min_idx];

    // removes the chosen request overwriting it with the last request in the queue 
    queue[min_idx] = queue[queue_count - 1];
    queue_count--; // decreases the count so the next request will overwrite the last one, already copied to the chosen position
    printf("WORKER %lu processing %s\n",
       pthread_self(),
       chosen.request.filename);
    return chosen;
}


static void process_request(client_request_t *req) {
    uint8_t hash[SHA256_DIGEST_LENGTH];
    //send file name and a shared variable "hash" 

    cache_get_or_compute(req->filename, hash);  

    int clientFifo = open(req->client_fifo_path, O_WRONLY);
    if (clientFifo == -1) {
        perror("could not open client fifo");
        return;
    }
    //sends the computed hash to the client
    write(clientFifo, hash, SHA256_DIGEST_LENGTH);
    close(clientFifo);
}

// worker loop continuously executed by any worker

static void *worker_loop(void *arg) {
    while (1) {
     
      //lock the queue mutex so that no other thread can choose the same request
        pthread_mutex_lock(&queue_mutex); 
        //wait if there are no requests 
        while (queue_count == 0) {
            pthread_cond_wait(&queue_not_empty, &queue_mutex);
        }
        pending_request_t job = dequeue_smallest();
        //unlock the queue before processing the request (that will be in state ENTRY_IN_PROGRESS until the end of computation) 
        //so that other threads can take other requests from the queue meanwhile 

        pthread_mutex_unlock(&queue_mutex);

        process_request(&job.request);
    }
    return NULL;
}

//creates MAX_THREADS threads

void threadpool_init(void) {
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_t tid;
        pthread_create(&tid, NULL, worker_loop, NULL);
        pthread_detach(tid);
    }
}
// adds a request to the shared queue (called by the thread that reads the FIFO)
void threadpool_submit(client_request_t request, long file_size) {
    pthread_mutex_lock(&queue_mutex);
    if (queue_count < MAX_QUEUE) {
        queue[queue_count].request = request;
        queue[queue_count].file_size = file_size;
        queue_count++;
        pthread_cond_signal(&queue_not_empty);
    } else {
        fprintf(stderr, "Queue full, dropping request for %s\n", request.filename);
       
    }
    pthread_mutex_unlock(&queue_mutex);
}
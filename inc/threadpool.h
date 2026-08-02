#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "common.h"

// creates MAX_THREADS workers
void threadpool_init(void);

// adds a request to the shared queue (called by the thread that reads the FIFO)
//the shared queue is written by the main server and read by the worker threads

void threadpool_submit(client_request_t request, long file_size);

#endif
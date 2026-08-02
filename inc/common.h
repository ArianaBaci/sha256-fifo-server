#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h>

#ifndef COMMON_H
#define COMMON_H

// costants and data structures 

#define MAX_PATH 256
#define MAX_THREADS 1
#define MAX_CACHE_ENTRIES 1000
#define PATH2SERVERFIFO "/tmp/server_fifo"
#define MAX_QUEUE 100
//permissions to access fifo
#define FIFO_PERMISSIONS 0666

typedef struct {
    char filename[MAX_PATH];          
    char client_fifo_path[MAX_PATH];  
} client_request_t;

typedef struct {
    client_request_t request;
    off_t file_size;
} pending_request_t;

typedef enum {
    ENTRY_IN_PROGRESS,
    ENTRY_READY
} entry_state_t;

typedef struct {
    char filename[MAX_PATH];
    uint8_t hash[SHA256_DIGEST_LENGTH];
    entry_state_t state;     
    pthread_cond_t ready;
} cache_entry_t;


#endif

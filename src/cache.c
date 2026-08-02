#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cache.h"
#include "common.h"
#include "digest.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <openssl/sha.h>

static cache_entry_t *cache[MAX_CACHE_ENTRIES];
static int cache_count = 0;
static pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

//looks for an entry in the cache with the given filename, returns NULL if not found
//needs cache_mutex lock
static cache_entry_t *find_entry(const char *filename) {
    for (int i = 0; i < cache_count; i++) {
        if (strcmp(cache[i]->filename, filename) == 0) {
            return cache[i];
        }
    }
    return NULL;
}

void cache_get_or_compute(const char *filename, uint8_t *hash) {

    pthread_mutex_lock(&cache_mutex);                               // lock the cache mutex

    cache_entry_t *entry = find_entry(filename);                    // look for an entry in the cache with the given filename

    if (entry == NULL) {                                            //no entry found, create a new one and compute the hash
       
        if (cache_count < MAX_CACHE_ENTRIES) {             // if the cache is not full, create a new entry
        entry = malloc(sizeof(cache_entry_t));
        strncpy(entry->filename, filename, MAX_PATH - 1);
        entry->filename[MAX_PATH - 1] = '\0';
        entry->state = ENTRY_IN_PROGRESS;        // set the state to in progress before releasing the lock (so other threads can use the cache but know the result id not ready to be used yet)
        pthread_cond_init(&entry->ready, NULL);  

        cache[cache_count++] = entry;

        pthread_mutex_unlock(&cache_mutex);  // release the lock before computing the hash
              } 
        else 
               {
        fprintf(stderr, "Cache full!\n");           //if cache is already full
        // use a simple algorithm (choose a random to evict an entry from the cache)
        int idx = rand() % cache_count;
        int n=MAX_CACHE_ENTRIES; //number of attempts to find an entry that is not in progress
        //avoid to ovverwrite an entry that is still being computed by another thread
        //n is chosen to be MAX_CACHE_ENTRIES to ensure not to make more attempts than a linear scan, but also to avoid chosing more often the first entries
        while(cache[idx]->state == ENTRY_IN_PROGRESS) {
            idx = rand() % cache_count;
            n--;
            if(n==0){
                //if all entries are in progress, compute the hash without caching it
                pthread_mutex_unlock(&cache_mutex);
                uint8_t computed[SHA256_DIGEST_LENGTH];

                digest_file(filename, computed);

                memcpy(hash, computed, SHA256_DIGEST_LENGTH);
                return;
            }
        }
        entry = cache[idx];             //if an entry is found that is not in progress, evict it and use it for the new entry
        strncpy(entry->filename, filename, MAX_PATH - 1);
        entry->filename[MAX_PATH - 1] = '\0';
        entry->state = ENTRY_IN_PROGRESS;        // set the state to "in progress before releasing the lock
        pthread_cond_init(&entry->ready, NULL);
        pthread_mutex_unlock(&cache_mutex);  // release the lock before computing the hash
    }

        // compute the hash of the file and store it in the created/chosen cache entry

        uint8_t computed[SHA256_DIGEST_LENGTH];
        printf("<Server> processing request for file %s\n", filename);
        digest_file(filename, computed);

        pthread_mutex_lock(&cache_mutex);           //lock the cache mutex again to update the cache entry with the computed hash and set the state to ready
        memcpy(entry->hash, computed, SHA256_DIGEST_LENGTH);
        entry->state = ENTRY_READY;              // set the state to ready after computing the hash
        pthread_cond_broadcast(&entry->ready);
        pthread_mutex_unlock(&cache_mutex);

        memcpy(hash, computed, SHA256_DIGEST_LENGTH);
        return;
    }

    if (entry->state == ENTRY_IN_PROGRESS) {
        printf("<Server> waiting for another thread that is already computing hash for file %s\n", filename);
       // some other thread is already computing the hash, wait for it to finish and take its result
        while (entry->state != ENTRY_READY) {
            pthread_cond_wait(&entry->ready, &cache_mutex);
        }
    }
    //when state gets to ENTRY_READY, end while loop and copy the hash to the shared variable "hash"
    memcpy(hash, entry->hash, SHA256_DIGEST_LENGTH);
    printf("<Server> using cached hash for file %s\n", filename);

    pthread_mutex_unlock(&cache_mutex);
}
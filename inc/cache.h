#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

// returns the SHA256 hash of the file with the given filename, either from cache or by computing it
// in the shared variable "hash" (which must be allocated by the caller)
void cache_get_or_compute(const char *filename, uint8_t *hash);

#endif
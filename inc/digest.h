#ifndef DIGEST_H
#define DIGEST_H

#include <stdint.h>

int digest_file(const char *filename, uint8_t *hash);

#endif
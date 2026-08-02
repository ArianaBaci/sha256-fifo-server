#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "errExit.h"
#include <stdlib.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <errno.h>
#include "common.h"

int main (int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];

    char path2ClientFIFO[MAX_PATH];
    snprintf(path2ClientFIFO, MAX_PATH, "/tmp/client_fifo_%d", getpid());

    if (mkfifo(path2ClientFIFO, FIFO_PERMISSIONS) == -1 && errno != EEXIST) {
        errExit("mkfifo client failed");
    }

   // printf("<Client> opening server FIFO to send the file %s...\n", filename);
    int serverFIFO = open(PATH2SERVERFIFO, O_WRONLY);
    if (serverFIFO == -1) {
        errExit("open failed");
    }

    client_request_t request;
    strncpy(request.filename, filename, MAX_PATH - 1);
    request.filename[MAX_PATH - 1] = '\0';
    strncpy(request.client_fifo_path, path2ClientFIFO, MAX_PATH - 1);
    request.client_fifo_path[MAX_PATH - 1] = '\0';

    if (write(serverFIFO, &request, sizeof(request)) != sizeof(request))
        errExit("write failed");

    if (close(serverFIFO) != 0)
        errExit("close failed");

   // printf("<Client> waiting for response...\n");
    int clientFIFO = open(path2ClientFIFO, O_RDONLY);
    if (clientFIFO == -1) {
        errExit("open client fifo failed");
    }

    uint8_t hash[SHA256_DIGEST_LENGTH];
    ssize_t r = read(clientFIFO, hash, SHA256_DIGEST_LENGTH);
    if (r != SHA256_DIGEST_LENGTH) {
        errExit("read failed");
    }

    printf("<Client> SHA256 hash of file %s: ", filename);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    if (close(clientFIFO) != 0)
        errExit("close failed");

    unlink(path2ClientFIFO);

    return 0;
}
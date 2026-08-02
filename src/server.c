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
#include "digest.h"
#include "threadpool.h"
#include "errExit.h"

// the file descriptor for the FIFO
int serverFIFO;

int main (int argc, char *argv[]) {
   
    printf("<Server> Makin2g FIFO...\n");

    // Make a FIFO with the permissons 0666 (read and write for everyone) 


    if (mkfifo(PATH2SERVERFIFO, FIFO_PERMISSIONS) == -1)
        if( errno != EEXIST){
        errExit("mkfifo failed");
        }

    printf("<Server> FIFO %s created!\n", PATH2SERVERFIFO);

    // Wait for clients in read-only mode. The open blocks the calling process
    // until another process opens the same FIFO in write-only mode

    printf("<Server> waiting for a client...\n");
    serverFIFO = open(PATH2SERVERFIFO, O_RDONLY);

    if (serverFIFO == -1)
        errExit("open failed");
    threadpool_init(); // initialize the thread pool
    
    //keep reading requests from the FIFO

    while (1) {

        client_request_t request;
        ssize_t bytesRead = read(serverFIFO, &request, sizeof(client_request_t));
        if (bytesRead == -1) {
        errExit("read failed");
        }
         if (bytesRead == 0) {
           continue;
    }

    if (bytesRead != sizeof(client_request_t)) {
        fprintf(stderr, "Warning: partial read (%zd bytes), skipping\n", bytesRead);
        continue;
    }
fflush(stdout);
   struct stat st;
    if (stat(request.filename, &st) == -1) {
     fprintf(stderr, "Cannot stat file %s\n", request.filename);
     continue;
    }
long file_size = st.st_size;
threadpool_submit(request, file_size);
    }

    // Close the FIFO
    if (close(serverFIFO) != 0)
        errExit("close failed");

    printf("<Server> removing FIFO...\n");
    // Remove the FIFO
    if (unlink(PATH2SERVERFIFO) != 0)
        errExit("unlink failed");
}

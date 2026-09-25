# SHA-256 Client-Server Computation Tool

A client-server application for computing SHA-256 hashes of files.
 Requests are exchanged through FIFOs. The server processes requests using a thread pool and stores previously computed hashes in an in-memory cache to avoid unnecessary computations.

This project was developed for the Operating Systems course at the University of Verona during the academic year 2025/2026.

## How It Works

1. A client receives a file path as a command-line argument.
2. The client sends the request to the server through a shared FIFO.
3. The server adds the request to a queue.
4. A worker thread processes the request.
5. The cache is checked before computing the hash.
6. If the hash is not cached, it is computed from the file contents.
7. The result is sent back to the client through a dedicated FIFO.

When multiple clients request the hash of the same file at the same time, only one thread performs the computation. The other threads wait for the result and reuse the cached value.

## Project Structure

```text
.
├── src/
│   ├── server.c       # Server implementation
│   ├── client.c       # Client implementation
│   ├── digest.c       # SHA-256 computation
│   ├── cache.c        # Hash cache and synchronization
│   ├── threadpool.c   # Worker thread pool and request queue
│   └── errExit.c      # Error handling
├── inc/
│   └── *.h            # Shared declarations and data structures
├── testOrdineEsecuzione.sh
├── testCaching.sh
└── README.md
```

## Configuration

The main configuration constants are defined in `common.h`, including:

- Maximum number of worker threads
- Maximum number of cache entries
- Server FIFO path
- FIFO permissions


## Tests

The project includes scripts for testing the main functionalities.

Test request ordering by file size:

```bash
./testOrdineEsecuzione.sh
```

Test cache usage with multiple simultaneous requests:

```bash
./testCaching.sh
```


## Compilation and Execution

Compile the project using the provided Makefile.

```bash
make
```
Start the server:

```bash
./server
```

In another terminal, start a client by providing the name of a file in same folder:

```bash
./client <filename>
```
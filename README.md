# In-Memory Key-Value Store

A from-scratch, event-driven in-memory key-value store written in C++ — a single-threaded server built around non-blocking I/O and `epoll`-based multiplexing, with its own incremental wire-protocol parser.

The goal of this project isn't to reproduce an existing database, but to work through the systems problems any high-performance networked store has to solve: TCP socket programming, I/O multiplexing, stream-oriented protocol parsing, and modular server architecture — building each piece up from raw sockets rather than a framework.

The wire protocol is RESP (REdis Serialization Protocol), which means the server is interoperable with the standard `redis-cli` out of the box — useful as a ready-made, protocol-correct client for manual testing, with no custom client needed.

## Features

- **Event-driven networking** — single-threaded event loop built on `epoll`, no thread-per-connection
- **Non-blocking I/O** — sockets configured with `O_NONBLOCK`; correctly handles `EAGAIN`/`EWOULDBLOCK`
- **Multi-client support** — many simultaneous connections handled on one thread
- **Per-client connection state** — each client owns an independent input buffer, correctly handling partial reads and multiple pipelined commands per read
- **Binary-safe protocol parsing** — incremental parser for length-prefixed array/bulk-string framing, resuming cleanly across partial reads
- **Command execution** — `SET`, `GET`, `DEL` backed by an in-memory `unordered_map`
- **Modular architecture** — clean separation between networking, connection management, protocol parsing, and command execution

## Architecture

```
                          client
                             |
                       TCP Byte Stream
                             |
                             v
===============================================
              Networking Layer
===============================================
   Socket -> Non-blocking -> epoll -> Connection Manager
                             |
                             v
===============================================
               Session Layer
===============================================
        Client Object (fd + input buffer)
                             |
                             v
===============================================
              Protocol Layer
===============================================
         TCP Stream Assembly -> Wire Decoder
                             |
                             v
===============================================
             Application Layer
===============================================
        Command Executor -> In-Memory Store
```

### Why this design

TCP guarantees an ordered byte stream, not message boundaries — a single command can arrive split across multiple `recv()` calls, and multiple commands can arrive concatenated in one call. Handling this correctly required:

- Moving from blocking, one-client-at-a-time handling to a non-blocking `epoll` event loop
- Giving each connection its own persistent state (`fd` + `input_buffer`) that survives between `epoll_wait()` calls
- Replacing naive line-based parsing with an incremental parser that reads exactly as many bytes as are available, returns early on incomplete input, and resumes cleanly on the next read

## Benchmarking results
Performance

GET (100 clients)
-----------------
78,818 ops/sec

SET (100 clients)
-----------------
36,767 ops/sec

10 KB payload
-------------
4,925 ops/sec

Latency (GET)

Average
790 µs

P99
2.6 ms



## Getting Started

### Prerequisites

- Linux (uses `epoll`, not portable to macOS/Windows without changes)
- A C++ compiler supporting C++17 or later (`g++`)

### Build

```bash
g++ -std=c++17 -o server *.cpp
```

> After `cd server`

### Run

```bash
./server <port>
```

### Connect

Using `redis-cli` (works out of the box, since the server speaks the same wire protocol):

```bash
redis-cli -p <port>
```

Or manually, using `nc`:

```bash
nc localhost <port>
```

### Example session

```
127.0.0.1:<port>> SET name Steve
OK
127.0.0.1:<port>> GET name
"Steve"
127.0.0.1:<port>> DEL name
(integer) 1
```

## Roadmap

- [ ] Persistence (write-ahead log / snapshotting)
- [ ] TTL and key expiration (lazy + active)
- [ ] Output buffering / partial write handling
<br>
and more

## Development Log

A detailed build log covering the socket lifecycle, the move to `epoll`, per-client state design, and protocol parsing decisions is maintained here: [`my_log.md`](./my_log.md)


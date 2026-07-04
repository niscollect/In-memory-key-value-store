
Very first step towards building an in-memory key-value-pair store, is reading about how Redis works, not deeply, just want it does and how(overview).
Describing very briefly, Redis server sits between a Client and a memory(normally a DB) working as a cache. Now when we zoom in on the server part (i.ee actual Redis), we find it operating on and across several layers: 
- **Network Layer** : TCP Sockets {How does a client talk to redis}
- **Protocol Layer** : {How do raw bytes become command}
- **Command Layer** : {Once we know it's "SET", what happens}
- **Storage Layer** : {Where does data live?}
- **Background Layer** : {Who cleans expired keys?, Who writes logs?, etc.}

Moving next, as we just saw, the very primary thing we now need is understanding of Network internals and their programming.
[Beej's Guide to Network programming] - this is what I used 

<hr>

<h3> STEP-1</h3>
#### Setup a Command Server
And also a client of course.

Let's break this into phases:

**Phase 1 — Understanding the socket lifecycle** 
Structure a basic server and client. 
Learn the two-socket model: listening socket vs connected socket. (very important)
Beware of usage of `sockfd` instead of `new_fd`.

**Phase 2 — Making it production-quality** 
Fix the skeleton systematically:

- `getaddrinfo()` return checked. Every return must be checked for failure(s).
- Note: Linked list must be iterated instead of assuming first node contains everything we need
- `SO_REUSEADDR` before `bind()`, always
- Every syscall return value checked (as said earlier)
- `freeaddrinfo()`, `close()` usage at the right places, is something to mind
- do mind the position of addr_size resetting, coz accept() can alter it

**Phase 3 — First working server/client** 
Compile and Verify.

**Phase 4 — Persistent connections** 
Move from single-shot to `while(1)` loop. Then to two-loop structure: outer accepts clients, inner handles commands from one client. 
NOTE:  `recv()` return 0 must be checked in the client side.

**Phase 5 — Command server** 
Ported to C++ (Yes, initially everything up till now, should be tried and tested in C, in Linux/WSL environment). May manual KV array (in C), but replace with with `std::unordered_map`, since that's the closest to the best we can make right now. Build SET, GET, DEL with proper error responses. 
NOTE: Use `strtok` for command parsing with `\r\n` scrubbing.

code's here: [[server.cpp (v1)]] , [[client.cpp (v1)]]

<hr>

Few observations before moving forward:
1) Currently we are assuming the entire message is received at once. But in actual `one recv === one command` is false. TCP guarantees 'ordered bytes' not 'boundary preservation'. Redis uses RESP and parser buffer for this.
2) We have used `strtok()` which is destructive in nature, i.e. it distorts the original buffer received. Fine for now.
3)  We are not handling SIGPIPE right now. We need to handle it, before it handles us.
4) Not handling "internal buffer full"
5) SET name Clark works, but SET name Clark Kent won't work. - -- handle that

<hr>

## STEP-2

**Multiple Clients**

Check if the server can handle multiple clients.

However, a point to establish the base:
Whole point of an im-memory key-value server store like Redis is that clients share the database. And so it's not a conflict if A sets a value and B changes or deletes it.

We've come to a realization, that the server, currently, can't handle multiple clients. The reason is, it's waiting on the first client to disconnect and not serving other clients waiting in the queue.
One solution is Multi threading. But, we won't do it, infact, Redis doesn't do it either. Redis uses single threaded event loop and non-blocking I/O (multiplexing)

So, we'll first learn How Multiple Clients Behave In a Single-Threaded Model.
[[https://stackoverflow.com/questions/51587316/how-to-handle-multiple-clients-on-single-thread-server-with-sockets]]
few things came around:
1) Non-block sockets and select calls
2) select()
3) event loops

-> Read about epoll. That's the solution

<hr>

**Supporting Multiple Clients**

The first implementation could only serve one client at a time.

Although multiple clients could connect to the server, the server entered the command-processing loop of the first client and never returned to `accept()` until that client disconnected. Every other client simply waited in the kernel's accept queue.

Initially, I considered introducing threads, but that would diverge from Redis' design. Redis instead relies on a single-threaded event loop with I/O multiplexing, so I decided to understand that architecture first.

#### Research
Spent some time reading about
- blocking vs non-blocking sockets
- I/O multiplexing
- `select()`
- `poll()`
- `epoll()`
Settled on learning `epoll`, since that's what Redis uses on Linux.

<hr>
### Refactoring the server

The original structure
```
accept()    
↓
while(client connected)    
	recv()
```
was replaced with an event loop.

Current flow:
```
create epoll instance
↓
register listening socket
↓
epoll_wait()
↓
if listening socket ready    
	accept new client    
	register client with epoll
↓
if client socket ready    
	recv()    
	process command
```

The server no longer blocks on a single client.

---

### NOTE

One thing that confused me initially:
```
int ready = epoll_wait(...);
```
does **not** return a file descriptor.

Instead, it returns the **number of ready events**, while the supplied `events` array gets populated with information about those descriptors.

Another small realization:
```
events.data()
```
returns a pointer to the first element of the vector, which is exactly what `epoll_wait()` expects.

---

### Client lifecycle
There are now three possible cases inside the event loop.
- Event belongs to the listening socket
    - `accept()`
    - register new client with epoll
- Event belongs to an existing client
    - `recv()`
    - execute command
    - `send()` response
- `recv()` returns `0`
    - remove socket from epoll
    - `close()` it

<hr>

### Current status
- Single-threaded server
- Multiple simultaneous clients
- Shared in-memory `unordered_map`
- Clients can connect and disconnect independently

---

### Remaining work

- Convert sockets to non-blocking mode
- Learn why `EAGAIN` is expected
- Stop assuming one `recv()` equals one command
- Handle partial `send()`
- Move towards edge-triggered `epoll`


----
## STEP-3

**Making the sockets non-blocking is quite critically important**

Imagine having Client-A and Client-B (already connected, just not sending anything yet). The server is *sleeping in `epoll_wait()`*. Suppose A wants to send "Hello", but TCP being TCP only accepts "He" for now, and "He" reaches the kernel so the kernel receiver buffer has "He" from A and nothing from B. Now epoll checks the socket has data, and wakes up the thread, our code starts reading `recv(fd, ...)`; 
`recv()` returns 2 (kernel had just 2 bytes); the kernel buffer is empty now and the program has "He" now.
Now the program inside has a while(1), inside which we have recv. when the next loop goes to recv, it again tries to ask if the socket has something to recv, but the kernel has nothing yet. Now "`rev()`" being a *blocking-call*, blocks. 
The thread *sleeps in `recv()`*
It never reaches back to epoll_wait() and so even if client-B sends something, the program will never know (even when the kernel has received it), coz it's sleeping in recv.

So, we need to make them unblocking. [sockets are blocking by default].
Use: 
```c++
int flags = fcntl(fd, F_GETFL, 0); // get the previous existing flags
fcntl(fd, F_SETFL, flags | O_NONBLOCK); // set the flags with a NON_BLOCKING flag
```



And as a very natural next step is to handle returns. While in blocking-calls, returning -1 meant error, in non-blocking calls -1 is expected,. it means there's more to come. So we utilize some errno, namely `EAGAIN` and `EWOULDBLOCK`, with -1 return. 
If -1 returns with errno = `ECONNRESET` or `ENOTCONN`, then those are actual errors

here's the new server: [[server.cpp (v2)]]

<br>
<hr>
<br>
## Step-4
**Modularity**

Now comes an interesting part. Our server is currently monolithic. One server.cpp handling everything.
It's better to have some modulation, on the basis of responsibilities.
Eventually making the server look like:
```C++
#define _POSIX_C_SOURCE 200112L
#include "server.h"
int main(int argc, char *argv[])
{
    // DB
    unordered_map<string, string> db;

    // normal args check
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int sockfd = create_listening_socket(argv[1]);
    if(sockfd == -1)
    {
        return -1;
    }
    // setting up event loop (epoll)
    event_loop(sockfd, db);
        // connection management (includes parsing and execution) - inside event_loop function

    close(sockfd);

    return 0;
}
```

<hr>
<br>
## STEP-5
**Understand TCP as a byte stream**

TCP guarantees order, not boundaries.
So for one command, you might receive "SET na" at first, and then "me John".

So we handle it using an input buffer.

Also, one important thing to think of.
We might have more than one clients, and everyone might be sending their incomplete info, as we have non-blocking sockets. So, to overcome that challenge, we need to have a buffer for each client.
Hence, we introduce a `client` object, to hold the state of a client. So, each client is essentially a client object holding the state.
```
obj client ->   fd, input_buffer
```

It's a necessity, not an OOP demand.


Now, let's think. The db (currently) is just an unordered_map, holding shared data.
But, how do we hold the clients?

But why do we need to hold the clients at the first place? Coz, say when a client gived "SET na", and stops, then the control gets back to epoll_wait, and by now the program is not obliged to remember that client (or any client), so when it again sends "me John", we need to find which client we have to append this to.

One way -> since `epoll_wait()` returns fd, we can have an `unordered_map<fd, Client>` which would give me the client with a quick lookup.
Think: 
	client connects -> create client and store ->umap is efficient
	client sends something -> we have fd, we need to lookup -> umap is efficient
	client disconnects -> deletion -> umap is efficient
Overall great choice.

Now, a natural question arises: which module should own the responsibility of handling the Clients.
The `server` or the `connection` module?
Only argument for `sever` could be that many modules might need them, so server being the orchestrator should handle it.
However, if though critically, within the system modules above `connection` module in the data-flow hierarchy, wouldn't need them, not presently at least.
So we'll let `connection` handle it.

Once done with this. Let's head towards `parser` to setup the parsing logic.

Let's have a rough algorithm.
So, we have an `input_buffer` to which we just append the next received *bytes*.
So, if first it brought "`SET na`" and then later it brough "`me John\nGET na`", then I have "`SET name John\nGET na`" in the `input_buffer` . 
So we want the algo to extract whatever exists before `\n`.
Also, mind that, since there could be more than one command even before the previous one is processed, we'll at some point have "`SET name Alex\nGET name John\nSET name Sam\nDEL na`"
So now we can see, the parser doesn't just have to extract whatever it before `\n`, it needs to loop over it too.



> **Future improvement:** Introduce per-client output buffers and move `send()` into the connection layer when supporting partial writes.



## STEP-6
**RESP**

For now, let's move on to implement RESP.
First study about RESP.
[[https://redis.io/docs/latest/develop/reference/protocol-spec/]] 

I'm just going to abandon my client program now. 
I need to worry about the server handling the RESP encoded statements, not about how to encode them.
I'll just use `redis-cli -p <my_server_port>`, and the official `redis-cli` does the work of proper RESP-compliant client.
So, finally here, I abandon my `client.cpp` program. 

<hr>

<br>

![Architecture diagram](./Pasted%20image%2020260703142843.png)


So far, so good.

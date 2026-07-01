#define _POSIX_C_SOURCE 200112L

#include <iostream>
#include <unordered_map>
#include <bits/stdc++.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/epoll.h>


using namespace std;

// NOTE: the namespace std line should be used before including any of the artifical header file. coz compiler would then go directly into those files, before reading the using namespace std, and would hit an error eventually
// However, it's a better practice to keep this namespace std line, in every header file

#include "connection.h"
#include "event_loop.h"
#include "executor.h"
#include "network.h"
#include "parser.h"



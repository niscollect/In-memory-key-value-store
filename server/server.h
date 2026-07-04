#define _POSIX_C_SOURCE 200112L

#include <iostream>
#include <fstream> // Required header for file handling
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


class ServerState
{
    public:
    // DB
    unordered_map<string, string> db;

    
    // Instantiate a stream object
    std::ofstream wal_file;

    ServerState()
    {
        this->wal_file.open("wal.txt", std::ios::app);
    }
};


const int CLIENT_DISCONNECTED = -2;
const int NETWORK_ERROR = -1;
const int SUCCESS = 0;


// NOTE: the namespace std line should be used before including any of the artifical header file. coz compiler would then go directly into those files, before reading the using namespace std, and would hit an error eventually
// However, it's a better practice to keep this namespace std line, in every header file

#include "connection.h"
#include "event_loop.h"
#include "executor.h"
#include "network.h"
#include "parser.h"

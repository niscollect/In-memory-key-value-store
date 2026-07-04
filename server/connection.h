#pragma once //* Prevents the header from being included multiple times

using namespace std;
class Client
{
    public:

    int fd;
    string input_buffer;

    Client() = default;
    //* we need it.
    //* We need a default consturctor, coz when we are creating the umap in connection.cpp with client as the value, for any creation of new key-value pair (since using umap[newfd], if newfd doesn't exists a pair is created in umap) creates an empty default client (which our current Client(int) won't allow if we don't ask explicitly this way) 
    // learnt the hard way
    
    Client(int fd)
    {
        this->fd = fd;
    }
};


void connect_to_client(int sockfd, int epfd, int num_of_ready_fds, struct sockaddr_storage their_addr, vector<epoll_event>& events, ServerState &state);
#pragma once //* Prevents the header from being included multiple times

using namespace std;
class Client
{
    public:

    int fd;
    string input_buffer;
    
    Client(int fd)
    {
        this->fd = fd;
    }
};


void connect_to_client(int sockfd, int epfd, int num_of_ready_fds, struct sockaddr_storage their_addr, vector<epoll_event>& events, unordered_map<string, string>& db);
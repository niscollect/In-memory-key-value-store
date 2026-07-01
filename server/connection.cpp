#include "server.h"

void connect_to_client(int sockfd, int epfd, int num_of_ready_fds, struct sockaddr_storage their_addr, vector<epoll_event>& events, unordered_map<string, string>& db) // Optimization: passing the "events" vector by reference, not by value
{
    for (int i = 0; i < num_of_ready_fds; i++)
    {
        // which fd created this event
        int fd = events[i].data.fd;

        // if(new_connection) call accept(), i.e.
        //* CASE A: The listening socket became ready
        if (fd == sockfd) // if the fd is sockfd means it's a new connection that has come; becoz now the listening socket is readable
        {
            socklen_t addr_size = sizeof their_addr;
            int new_fd = accept(sockfd, (sockaddr *)&their_addr, &addr_size); // client_fd

            // got the client
            if (new_fd == -1)
            {
                perror("server: accept");
                // break;
                continue;
            }

            //* make the client_socket also non-blocking (only after checking that it succeeded)
            if (!make_non_blocking(new_fd))
            {
                perror("server: fcntl");
                break;
            }

            cout << "accepted " << new_fd << endl;

            //* add the client to epoll
            epoll_event client_ev{};
            client_ev.events = EPOLLIN;
            client_ev.data.fd = new_fd;
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &client_ev) == -1)
            {
                perror("epoll_ctl");
                // break;
                continue;
            }

            // now epoll will watch this client also, and tell us if it sends data
        }

        // if(client_data) call recv()
        //* CASE B: The client socket became ready
        //* since it is not sockfd (server_fd), it must be one of the the clients
        else
        {

            // command parsing
            parser(fd, epfd, db);

            // command execution - done by parser.cpp
            
        }
    }
}
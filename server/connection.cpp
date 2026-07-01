#include "server.h"

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

unordered_map<int, Client> clients;

void disconnect_client(){} // we'll utilize it shortly


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

            // create a client & insert it
            Client client(new_fd);
            clients[new_fd] = client;
            //! remember to delete this client, when it disconnects.
            //* Seeing clearly, client's disconnection is not found here, but in parser.cpp


            // now epoll will watch this client also, and tell us if it sends data
        }

        // if(client_data) call recv()
        //* CASE B: The client socket became ready
        //* since it is not sockfd (server_fd), it must be one of the the clients
        else
        {

            // command parsing
            int parser_result = parser(fd, db);

            if(parser_result == PARSER_DISCONNECTED) // means client disconnected
            {
                //* close the client and remove it from the epoll's watchlist -- since the client is already gone, no point in keeping it
                // also remove it from the `clients` umap
                close(fd);
                clients.erase(fd);
                if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1)
                {
                    perror("epoll_ctl");
                    // continue; // not break but continue, coz we have other clients too
                    // return;
                    
                    continue;
                }
            }
            if(parser_result == PARSER_ERROR) // means client abrupt failure
            {
                perror("Server: recv");
                close(fd);
                clients.erase(fd);
                if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1)
                {
                    perror("epoll_ctl");
                    // continue; // not break but continue, coz we have other clients too
                    // return;

                    continue;
                }
                // break;
                // continue; // not break but continue
                // return;

                continue;
            }

            // command execution - done by parser.cpp
            
        }
    }
}
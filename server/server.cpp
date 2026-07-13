#define _POSIX_C_SOURCE 200112L

#include "server.h"

int main(int argc, char *argv[])
{

    ServerState state;

    std::cout << "[Server] Booting up IMKVS..." << std::endl;
    std::cout << "[Server] Starting WAL recovery..." << std::endl;

    load_wal(state);

    std::cout << "[Server] Initialization complete." << std::endl;
    std::cout << "[Server] Ready to accept connections!" << std::endl;


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
    event_loop(sockfd, state);

        // connection management (includes parsing and execution) - inside event_loop function

        
    close(sockfd);

    return 0;
}
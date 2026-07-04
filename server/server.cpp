#define _POSIX_C_SOURCE 200112L

#include "server.h"


void load_wal(ServerState& state)
{
    bool is_recovery = true;
    
    std::ifstream inputFile("wal.txt");

    if (!inputFile.is_open())
    {
        std::cerr << "Error: Could not open the file!" << std::endl;
    }

    // std::string buff;

    Client fake_client(-1);

    // Read the entire stream buffer into a stringstream
    std::stringstream buffer;
    buffer << inputFile.rdbuf();

    // Convert the buffer into a single string variable
    fake_client.input_buffer = buffer.str();

    parser(fake_client, state, is_recovery);


    // Finally close the file (good practice)
    inputFile.close();


}


int main(int argc, char *argv[])
{

    ServerState state;

    load_wal(state);


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
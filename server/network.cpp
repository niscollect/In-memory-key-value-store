#include "server.h"

bool make_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
        return false;

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return false;

    return true;
}

int create_listening_socket(char *port)
{
    // server socket setup

    int sockfd;
    const int backlog = 10;

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int getaddrinfo_return = getaddrinfo(NULL, port, &hints, &res);
    if (getaddrinfo_return != 0)
    {
        // exit(1);
        return -1;
    }

    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (sockfd == -1)
        {
            perror("Server: socket");
            continue;
        }

        //* make the socket non-blocking (only after checking that it succeeded)
        if (!make_non_blocking(sockfd))
        {
            perror("server: fcntl");
            continue;
        }

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("setsockopt");
            close(sockfd);
            // exit(1);
            return -1;
        }

        int bind_return = bind(sockfd, p->ai_addr, p->ai_addrlen);
        if (bind_return == -1)
        {
            close(sockfd);
            perror("Server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind to any address\n");
        // return 2;
        return -1;
    }

    freeaddrinfo(res);

    // adding listening ability to the server socket (which is still essentially setting up the server)
    int listen_return = listen(sockfd, backlog);

    if (listen_return == -1)
    {
        perror("Server: listen");
        close(sockfd);
        // exit(1);
        return -1;
    }


    return sockfd;
}
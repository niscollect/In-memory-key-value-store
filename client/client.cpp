#define _POSIX_C_SOURCE 200112L

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

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int getaddrinfo_return = getaddrinfo(argv[1], argv[2], &hints, &res);

    if (getaddrinfo_return != 0)
    {
        exit(1);
    }

    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
        {
            perror("Client: socket");
            continue;
        }

        int connect_return = connect(sockfd, p->ai_addr, p->ai_addrlen);
        if (connect_return == -1)
        {
            perror("Client: connect");
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "client: failed to bind to any address\n");
        return 2;
    }

    freeaddrinfo(res);

    while (1)
    {
        char sentence[100]; // Create a buffer to hold 100 characters
        printf(">");
        if (fgets(sentence, sizeof(sentence), stdin) == NULL)
        {
            break;
        }
        //* fgets is a breaking call, so it stops the execution unless it receives an input

        int sent_bytes = send(sockfd, sentence, strlen(sentence), 0);
        if (sent_bytes == -1)
        {
            perror("client: send");
            close(sockfd);
            exit(1);
        }

        // NOW WAIT FOR THE SERVER TO BRING SOMETHING BACK
        char buff[1024];
        int buff_received = recv(sockfd, buff, (sizeof buff) - 1, 0);
        if (buff_received == 0)
        {
            printf("Server closed connection.\n");
            close(sockfd);
            exit(0);
        }
        if (buff_received == -1)
        {
            perror("client: recv");
            close(sockfd);
            return 1;
        }

        buff[buff_received] = '\0';
        printf("%s\n", buff);
    }

    close(sockfd);
    return 0;
}
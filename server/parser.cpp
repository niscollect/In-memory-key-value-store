#include "server.h"

void parser(int fd, int epfd, unordered_map<string, string>& db)
{
    char buff[1024];
    int buff_received = recv(fd, buff, (sizeof buff) - 1, 0);

    // client disconnected
    if (buff_received == 0)
    {
        cout << "Client disconnected: " << fd << endl;

        //* close the client and remove it from the epoll's watchlist -- since the client is already gone, no point in keeping it
        close(fd);
        if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1)
        {
            perror("epoll_ctl");
            // continue; // not break but continue, coz we have other clients too
            return;
        }

        // break;
        // continue; // not break but continue
        return;
    }

    // // if recv() failed
    // with non-blocking sockets, -1 is not a direct error, it's expected
    if (buff_received == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Not an error. It's expected behaviour
            // We've read all currently available data.
            // continue;
            return;
        }

        perror("Server: recv");
        close(fd);
        if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1)
        {
            perror("epoll_ctl");
            // continue; // not break but continue, coz we have other clients too
            return;
        }
        // break;
        // continue; // not break but continue
        return;
    }

    // reached here, means actual data has come. Parse it
    buff[buff_received] = '\0';
    printf("%s\n", buff);
    // parse commands
    // parse buff
    // take the first word and see if it's SET or GET
    // * Include "\r\n" in delimiters to scrub terminal input formatting
    char *command = strtok(buff, " \r\n");
    char *key = strtok(NULL, " \r\n"); //* You pass NULL after the first call to tell it "continue from where you left off."
    char *value = strtok(NULL, " \r\n");


    executor(fd, command, key, value, db);
}
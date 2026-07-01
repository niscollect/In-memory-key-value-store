#include "server.h"

int parser(int fd, unordered_map<string, string>& db)
{
    char buff[1024];
    int buff_received = recv(fd, buff, (sizeof buff) - 1, 0);

    // client disconnected
    if (buff_received == 0)
    {
        cout << "Client disconnected: " << fd << endl;

        // break;
        // continue; // not break but continue
        return PARSER_DISCONNECTED; // to indicate that cient has disconnected
    }

    // // if recv() failed
    // with non-blocking sockets, -1 is not a direct error, it's expected
    if (buff_received == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Not an error. It's expected behaviour

            //*? It means: I tried to read, but the buffer is empty right now. Go back to epoll_wait and try again later

            // We've read all currently available data.
            // continue;
            return PARSER_OK; // basically return 0 (normal)
        }

        // else, client abrupt failure
        return PARSER_ERROR;
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

    return PARSER_OK; // all good
}
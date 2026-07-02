#include "server.h"

int executor(int fd, string command, string key, string value, unordered_map<string, string> &db)
{

    //@note When server fails to send a response, the connection must close.
    // currently in our we are just returning it, which leaves back a zombie connection. We need to close it.
    // But that is the responsibility of the `connection` module

    if (command == "SET")
    {
        db[key] = value;
        if (send(fd, "OK\n", 3, 0) == -1)
        {
            perror("couldn't save");
            return NETWORK_ERROR;
        }
    }
    else if (command == "GET")
    {
        //* SAFETY CHECK: Check if key exists in the database
        if (db.count(key) > 0)
        {
            string response = db[key] + "\n";
            if (send(fd, response.c_str(), response.length(), 0) == -1)
            {
                perror("server: couldn't send    ---");
                return NETWORK_ERROR;
            }
        }
        else
        {
            const char *msg = "ERROR: Key Not Found\n";
            if (send(fd, msg, strlen(msg), 0) == -1)
            {
                perror("server: couldn't send-");
                return NETWORK_ERROR;
            }
        }
    }
    else if (command == "DEL")
    {

        // SAFETY CHECK: Check if key exists in the database
        if (db.count(key) > 0)
        {
            db.erase(key);
            if (send(fd, "OK\n", 3, 0) == -1) // inform the client that want he wanted has been done successfully
            {
                perror("server: send");
                return NETWORK_ERROR;
            }
        }
        else
        {
            const char *msg = "ERROR: Key Not Found\n";
            if (send(fd, msg, strlen(msg), 0) == -1)
            {
                perror("server: couldn't send-");
                return NETWORK_ERROR;
            }
        }
    }
    else
    {
        const char *msg = "ERROR: Unknown Command\n";
        if (send(fd, msg, strlen(msg), 0) == -1)
        {
            perror("server: send");
            // break;
            return NETWORK_ERROR;
        }
    }


    return SUCCESS;
}
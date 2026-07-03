#include "server.h"

int executor(int fd, string command, string key, string value, unordered_map<string, string> &db)
{

    //@note When server fails to send a response, the connection must close.
    // currently in our we are just returning it, which leaves back a zombie connection. We need to close it.
    // But that is the responsibility of the `connection` module

    if (command == "SET")
    {
        db[key] = value;
        if (send(fd, "+OK\r\n", 5, 0) == -1)
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
            string val = db[key];
            string response = "$" + to_string(val.length()) + "\r\n" + val + "\r\n";
            if (send(fd, response.c_str(), response.length(), 0) == -1)
            {
                perror("server: couldn't send    ---");
                return NETWORK_ERROR;
            }
        }
        else
        {
            const char *msg = "$-1\r\n"; // this means null for the RESP, i.e. key not found
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
            // 1 key deleted successfully
            if (send(fd, ":1\r\n", 4, 0) == -1) 
                return NETWORK_ERROR;
        }
        else
        {
            // 0 keys deleted (key didn't exist)
            if (send(fd, ":0\r\n", 4, 0) == -1)
                return NETWORK_ERROR;
        }

    }
    else
    {
        const char *msg = "-ERROR: Unknown Command\r\n";
        if (send(fd, msg, strlen(msg), 0) == -1)
        {
            perror("server: send");
            // break;
            return NETWORK_ERROR;
        }
    }


    return SUCCESS;
}
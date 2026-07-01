#include "server.h"


void executor(int fd, char *command, char *key, char *value, unordered_map<string, string>& db)
{
    //* SAFETY CHECK: If command is completely empty, skip parsing
    if (command == NULL)
    {
        //---------- close(fd);
        // continue;
        return;
    }

    if (strcmp(command, "SET") == 0)
    {
        //* SAFETY CHECK: Ensure a valid key and value exist
        if (key != NULL && value != NULL)
        {
            db[key] = value;
            if (send(fd, "OK\n", 3, 0) == -1)
            {
                // close(sockfd);
                perror("couldn't save");
                // break; //* breaking will close the connection anyways
                return;

                //-------------- close(fd);
            }
        }
        else
        {
            const char *msg = "ERROR: Missing Key or Value\n";
            if (send(fd, msg, strlen(msg), 0) == -1)
            {
                // close(sockfd);
                perror("server: couldn't send   -");
                // break; //* breaking will close the connection anyways
                return;

                //------------- close(fd);
            }
        }
    }
    else if (strcmp(command, "GET") == 0)
    {
        // * SAFETY CHECK: Ensure a valid key was passed
        if (key != NULL)
        {
            //* SAFETY CHECK: Check if key exists in the database
            if (db.count(key) > 0)
            {
                string response = db[key] + "\n";
                if (send(fd, response.c_str(), response.length(), 0) == -1)
                {
                    // close(sockfd);
                    perror("server: couldn't send    ---");
                    // break; //* breaking will close the connection anyways
                    return;

                    //----------- close(fd);
                }
            }
            else
            {
                const char *msg = "ERROR: Key Not Found\n";
                if (send(fd, msg, strlen(msg), 0) == -1)
                {
                    // close(sockfd);
                    perror("server: couldn't send-");
                    // break; //* breaking will close the connection anyways
                    return;

                    //----------- close(fd);
                }
            }
        }
        else
        {
            const char *msg = "ERROR: Missing Key\n";
            if (send(fd, msg, strlen(msg), 0) == -1)
            {
                perror("server: send");
                // break;
                return;
            }
        }
    }
    else if (strcmp(command, "DEL") == 0)
    {
        // SAFETY CHECK: Ensure a valid key was passed
        if (key != NULL)
        {
            // SAFETY CHECK: Check if key exists in the database
            if (db.count(key) > 0)
            {
                db.erase(key);
                if (send(fd, "OK\n", 3, 0) == -1) // inform the client that want he wanted has been done successfully
                {
                    perror("server: send");
                    // break;
                    return;
                }
            }
            else
            {
                const char *msg = "ERROR: Key Not Found\n";
                if (send(fd, msg, strlen(msg), 0) == -1)
                {
                    // close(sockfd);
                    perror("server: couldn't send-");
                    // break; //* breaking will close the connection anyways
                    return;

                    //----------- close(fd);
                }
            }
        }
        else
        {
            const char *msg = "ERROR: Missing Key\n";
            if (send(fd, msg, strlen(msg), 0) == -1)
            {
                perror("server: send");
                // break;
                return;
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
            return;
        }
    }
}
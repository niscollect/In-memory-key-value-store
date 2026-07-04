#include "server.h"

int executor(int fd, string command, string key, string value, ServerState& state, bool is_recovery)
{

    //@note When server fails to send a response, the connection must close.
    // currently in our we are just returning it, which leaves back a zombie connection. We need to close it.
    // But that is the responsibility of the `connection` module

    if (command == "SET")
    {
        if (!is_recovery) // do not write to WAL write reading from it
        {
            // * WAL HERE
            // save in RESP format -> just make it RESP
            if (state.wal_file.is_open())
            {
                state.wal_file << "*3\r\n"
                               << "$3\r\nSET\r\n"
                               << "$" << key.length() << "\r\n"
                               << key << "\r\n"
                               << "$" << value.length() << "\r\n"
                               << value << "\r\n";

                state.wal_file.flush();
                // force save to disk
            }
            else
            {
                std::cerr << "Error: Write-Ahead Log is not open!" << std::endl;
            }
        }

        state.db[key] = value;
        if (!is_recovery)
        {
            if (send(fd, "+OK\r\n", 5, 0) == -1)
            {
                perror("couldn't save");
                return NETWORK_ERROR;
            }
        }
    }
    else if (command == "GET")
    {
        //* SAFETY CHECK: Check if key exists in the database
        if (!is_recovery)
        {
            if (state.db.count(key) > 0)
            {
                string val = state.db[key];
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
    }
    else if (command == "DEL")
    {

        // SAFETY CHECK: Check if key exists in the database
        if (state.db.count(key) > 0)
        {
            if (!is_recovery)
            {
                // * WAL HERE
                // save in RESP format -> just make it RESP
                if (state.wal_file.is_open())
                {
                    state.wal_file << "*2\r\n"
                                   << "$3\r\nDEL\r\n"
                                   << "$" << key.length() << "\r\n"
                                   << key << "\r\n";

                    state.wal_file.flush();
                    // force save to disk
                }
                else
                {
                    std::cerr << "Error: Write-Ahead Log is not open!" << std::endl;
                }
            }

            state.db.erase(key);
            if (!is_recovery)
            {
                // 1 key deleted successfully
                if (send(fd, ":1\r\n", 4, 0) == -1)
                    return NETWORK_ERROR;
            }
        }
        else
        {
            if (!is_recovery)
            {
                // 0 keys deleted (key didn't exist)
                if (send(fd, ":0\r\n", 4, 0) == -1)
                    return NETWORK_ERROR;
            }
        }
    }
    else
    {
        if (!is_recovery)
        {
            const char *msg = "-ERROR: Unknown Command\r\n";
            if (send(fd, msg, strlen(msg), 0) == -1)
            {
                perror("server: send");
                // break;
                return NETWORK_ERROR;
            }
        }
    }

    return SUCCESS;
}
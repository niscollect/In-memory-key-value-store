#include "server.h"

// later didvide parseBuff into parse, validate and execute. Later when it grows large
int parseBuff(string statement, Client &client, unordered_map<string, string> &db)
{
    stringstream ss(statement);
    string command, key, value;

    ss >> command; // inserts the first word (extracts by seperation through spaces/tabs/delimiters)
    ss >> key;

    // now we can't just directly do ss >> value, coz the value might have spaces too: SET message Hello World
    // so we just take whatever is left there, in the stringstream

    getline(ss, value); // places chacaters from ss LHS into the RHS

    // Trim any leading space from getline and trailing \r
    if (!value.empty() && value.front() == ' ')
        value.erase(0, 1);
    if (!value.empty() && value.back() == '\r')
        value.pop_back();

    // also trim '\r' from 'key' just in case it was a GET command
    if (!key.empty() && key.back() == '\r')
        key.pop_back();

    if (command == "DEL" || command == "GET")
    {
        if (!value.empty())
        {
            const char *msg = "ERROR: Too Many Arguments\n";

            // 1. Check if the send failed (socket broken)
            if (send(client.fd, msg, strlen(msg), 0) == -1)
            {
                return NETWORK_ERROR;
            }

            // 2. We successfully sent the error. Now abort THIS command.
            return SUCCESS;
        }
    }

    //* SAFETY CHECK: If command is completely empty, skip parsing
    if (command.empty())
    {
        // continue;
        return SUCCESS;
        // just ignore blank lines. Keep moving
    }
    if (command == "SET")
    {
        //* SAFETY CHECK: Ensure a valid key and value exist
        if (key.empty() || value.empty())
        {
            const char *msg = "ERROR: Missing Key or Value\n";
            if (send(client.fd, msg, strlen(msg), 0) == -1)
            {
                perror("server: couldn't send   -");
                return NETWORK_ERROR;
            }

            return SUCCESS; // if we don't add this, we'll just end up calling the executor even when we've sent the failure message
        }
    }
    else if (command == "GET")
    {
        // * SAFETY CHECK: Ensure a valid key was passed
        if (key.empty())
        {
            const char *msg = "ERROR: Missing Key\n";
            if (send(client.fd, msg, strlen(msg), 0) == -1)
            {
                perror("server: send");
                return NETWORK_ERROR;
            }

            return SUCCESS;
        }
    }
    else if (command == "DEL")
    {
        //* SAFETY CHECK: Ensure a valid key was passed
        if (key.empty())
        {
            const char *msg = "ERROR: Missing Key\n";
            if (send(client.fd, msg, strlen(msg), 0) == -1)
            {
                perror("server: send");
                return NETWORK_ERROR;
            }

            return SUCCESS;
        }
    }

    return executor(client.fd, command, key, value, db);
}

int parser(Client &client, unordered_map<string, string> &db)
{
    string &buff = client.input_buffer;

    // parse commands
    // parse buff

    //      Step A: Is there at least one complete command?
    //      ↓
    //      If no:
    //          return
    //      ↓
    //      If yes:
    //          Extract ONE command.
    //      ↓
    //      Remove only that command from input_buffer.
    //      ↓
    //      Parse it.
    //      ↓
    //      Execute it.
    //      ↓
    //      Go back to Step A.

    // scan the buff for the first occurance of "\n"
    while (true)
    {
        size_t pos = buff.find("\n"); // position of first occurance of "\n"

        if (pos == string::npos)
            break;

        string statement = buff.substr(0, pos);
        buff.erase(0, pos + 1);

        int parseBuffRes = parseBuff(statement, client, db);

        if (parseBuffRes != SUCCESS)
            return parseBuffRes;

        // buff hold only the remaining data now
    }

    return SUCCESS; // all good
}
#include "server.h"

// TODO: handleCommand will now accept command, key, value, all 3 separately
// TODO: parser needs to be modulated well enough to decode the RESP message

// later didvide handleCommand into parse, validate and execute. Later when it grows large
int handleCommand(string command, string key, string value, Client &client, ServerState &state, bool is_recovery)
{
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
            const char *msg = "-ERROR: Too Many Arguments\r\n";

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
            const char *msg = "-ERROR: Missing Key or Value\r\n";
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
            const char *msg = "-ERROR: Missing Key\r\n";
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
            const char *msg = "-ERROR: Missing Key\r\n";
            if (send(client.fd, msg, strlen(msg), 0) == -1)
            {
                perror("server: send");
                return NETWORK_ERROR;
            }

            return SUCCESS;
        }
    }

    return executor(client.fd, command, key, value, state); 
    // pass the "RESP statement" to the executor, coz only the verified ones reach there, and so we write only the verified ones
}

int parser(Client &client, ServerState &state, bool is_recovery = false)
{
    string& buff = client.input_buffer;

    if (buff.empty()) return SUCCESS;

    // must start with *
    if(buff[0] != '*')
        return NETWORK_ERROR;

    while(true)
    {
        vector<string> statement;
        string command, key, value;


        // find the first command 

        int cursor = 0;

        size_t pos1 = buff.find("\r\n", cursor); // this is where we get the array length
        if(pos1 == string::npos)
            break;

        cursor = pos1 + 2;

        // by the hard rule of the protocol, we are going to have array of bulk strings only.

        // int arrayLen = stoi(buff.substr(1, pos1 - 1));
        //* NOTE: using stoi is good. But, NEVER TRUST THE CLIENT. If client sends *p instead of *3, stoi throws invalid-argument exeception
        // so to handle that, there're two ways: 1) enclose stoi in try-catch; 2) loop over every char and see if that's an ineteger; 3) [The C++17 way][High performance way] use std::from_chars; 4) strtol (the old ugly way)
        // we'll use the third method

        //* from_chars(start_pointer, end_pointer, output_variable)
        int arrayLen = 0;
        auto result = std::from_chars(buff.data() + 1, buff.data() + pos1, arrayLen);
        //* if it failed to read a number, or if the number was too big
        if (result.ec != std::errc())  
        {
            return NETWORK_ERROR; 
        }


        // there'll be arrayLen bulk strings to read
        // loop that many times and find \r\n to find $n
        for(int i = 0; i < arrayLen; i++)
        {
            if(buff[cursor] != '$')
                return NETWORK_ERROR; //? or should it be network error?

            size_t pos = buff.find("\r\n", cursor);
            if(pos == string::npos)
                return SUCCESS; // The complete command has not come yet

            // means we've got a $n
            // int stringLen = stoi( buff.substr(cursor+1, pos-cursor-1) );
            int stringLen = 0;
            auto result = std::from_chars(buff.data() + cursor + 1, buff.data() + pos, stringLen);
            if (result.ec != std::errc())  
            {
                return NETWORK_ERROR; //? or should it be network error? 
            }

            cursor = pos + 2;

            // read the actual string (DO NOT USE .find() otherwise it won't be binary safe)
            // check if the buffer has enough bytes to hold the stringLen-byte string + 2 trailing \r\n
            if(cursor + stringLen + 2 > buff.length())
                return SUCCESS; // incomplete string

            // before extracting, just check if it is even valid (it must end with CRLF)
            if(buff[cursor + stringLen] != '\r' || buff[cursor + stringLen + 1] != '\n')
            {
                return NETWORK_ERROR; //? or should it be network error?
            }

            // extract the string by its length
            statement.push_back(buff.substr(cursor, stringLen));
            
            cursor = cursor + stringLen + 2;
        }

        command = statement.size() > 0 ? statement[0] : "";
        key = statement.size() > 1 ? statement[1] : "";
        value = statement.size() > 2 ? statement[2] : "";

        int handleCommandRes = handleCommand(command, key, value, client, state, is_recovery);

        if(handleCommandRes != SUCCESS)
            return handleCommandRes;

        buff.erase(0, cursor); // remove the first statement; we just processed it
    }

    return SUCCESS;
}

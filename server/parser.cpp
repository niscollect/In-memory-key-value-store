#include "server.h"

int parser(Client& client, unordered_map<string, string>& db)
{
    string buff = client.input_buffer;
    
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




    return PARSER_OK; // all good
}
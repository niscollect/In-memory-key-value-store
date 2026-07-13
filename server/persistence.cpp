#include "server.h"

void load_wal(ServerState &state)
{
    bool is_recovery = true;

    std::ifstream inputFile("wal.txt");

    if (!inputFile.is_open())
    {
        std::cerr << "Error: Could not open the file!" << std::endl;
    }

    // std::string buff;

    Client fake_client(-1);

    // Read the entire stream buffer into a stringstream
    std::stringstream buffer;
    buffer << inputFile.rdbuf();

    // Convert the buffer into a single string variable
    fake_client.input_buffer = buffer.str();

    int result = parser(fake_client, state, is_recovery);

    if (result != SUCCESS)
    {
        std::cerr << "[FATAL] WAL file is corrupted! Recovery aborted." << std::endl;
        exit(1); // Stop the server from booting
    }


    // Finally close the file (good practice)
    inputFile.close();

    // The parser finished, so recovery is mathematically complete!
    std::cout << "[Server] DB loaded from WAL file." << std::endl;
    std::cout << "[Server] Recovered " << state.db.size() << " keys into memory." << std::endl;
}

void append_to_wal(ServerState &state, string command, string key, string value)
{
    if (command == "SET")
    {
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

    else if (command == "DEL")
    {
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
}
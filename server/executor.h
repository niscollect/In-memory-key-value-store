#pragma once //* Prevents the header from being included multiple times

using namespace std;

int executor(int fd,string command, string key, string value, ServerState &state, bool is_recovery = false);
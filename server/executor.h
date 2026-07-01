#pragma once //* Prevents the header from being included multiple times

using namespace std;

void executor(int fd, char *command, char *key, char *value, unordered_map<string, string>& db);
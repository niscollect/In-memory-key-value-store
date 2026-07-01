#pragma once //* Prevents the header from being included multiple times

using namespace std;

void executor(int fd, string command, string key, string value, unordered_map<string, string>& db);
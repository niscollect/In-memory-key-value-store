#pragma once //* Prevents the header from being included multiple times

#include <string>

using namespace std;

void parser(int fd, int epfd, unordered_map<string, string>& db);
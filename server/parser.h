#pragma once //* Prevents the header from being included multiple times

#include <string>

using namespace std;

int parser(int fd, unordered_map<string, string>& db);
// -2 = client disconnected (PARSER_DISCONNECTED)
// -1 = client abrupt failure (PARSER_ERROR)
// 0 = all good
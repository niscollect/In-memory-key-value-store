#pragma once //* Prevents the header from being included multiple times

#include <string>

using namespace std;

int parser(Client& client, ServerState &state, bool is_recovery = false);
// -2 = client disconnected (PARSER_DISCONNECTED)
// -1 = client abrupt failure (PARSER_ERROR)
// 0 = all good
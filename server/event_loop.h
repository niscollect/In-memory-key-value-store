#pragma once //* Prevents the header from being included multiple times

using namespace std;

void event_loop(int sockfd, unordered_map<string, string>& db);
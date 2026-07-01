#pragma once //* Prevents the header from being included multiple times

using namespace std;

bool make_non_blocking(int fd);
int create_listening_socket(char *port);
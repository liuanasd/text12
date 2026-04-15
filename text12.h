#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#define PORT_NUMBER 7856
#define BUF_SIZE 1024
#define NAME_SIZE 32

struct ClientInfo {
    int client_fd;
    std::string name;
};

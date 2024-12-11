#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>

class InetAddress {
public:
    InetAddress();
    InetAddress(const char *ip, const int port);
    ~InetAddress();

    struct sockaddr_in addr;
    socklen_t addr_sz;
};
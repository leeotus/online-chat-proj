#include "InetAddress.hpp"

#include <string.h>

InetAddress::InetAddress() : addr_sz(sizeof(addr))
{
    memset(&addr, 0, sizeof(addr));
}

InetAddress::InetAddress(const char* ip, const int port) : addr_sz(sizeof(addr))
{
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
}
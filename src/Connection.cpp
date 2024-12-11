#include "Connection.hpp"
#include "InetAddress.hpp"
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

Connection::Connection()
{
    perror("do not use this function directly!");
    exit(EXIT_FAILURE);
}

Connection::Connection(int _fd, InetAddress *_inetAddr) : fd(_fd), inetAddr(_inetAddr) {}

Connection::Connection(const char *ip, const int port)
{
    perror("don't use this function directly!");
    exit(EXIT_FAILURE);
}

Connection::~Connection()
{
    if(inetAddr!=nullptr)
    {
        delete inetAddr;
    }
    close(fd);
}

void Connection::setNonBlocking()
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}
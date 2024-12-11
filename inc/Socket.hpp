#pragma once

#include "Connection.hpp"

#define MAX_LISTEN_NUM 15

class Socket : public Connection
{
public:
    Socket();
    Socket(int fd, InetAddress* _inetAddr) = delete;
    Socket(const char *ip, const int port);

    void listen(int n = MAX_LISTEN_NUM);
    Connection* accept();

private:
    void bind();
};
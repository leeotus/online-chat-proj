#include "Epoll.hpp"
#include "log/util.hpp"
#include <unistd.h>

Epoll::Epoll()
{
    epfd = epoll_create(1);
    errif(epfd == -1, "epoll create failed!");
}

Epoll::~Epoll()
{
    close(epfd);
}

void Epoll::updateConnection(Connection *conn, EpollAction act)
{

}
#include "Epoll.hpp"
#include "log/util.hpp"
#include "Connection.hpp"

#include <unistd.h>
#include <vector>

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
    if(act == ACTION_UPDATE)
    {
        if(conn->getInEpoll())
        {
            epoll_ctl(epfd, EPOLL_CTL_MOD, conn->getFd(), conn->getEvent());
            conn->setNonBlocking();
        } else if(!conn->getInEpoll())
        {
            epoll_ctl(epfd, EPOLL_CTL_ADD, conn->getFd(), conn->getEvent());
            conn->setInEpoll();
            conn->setNonBlocking();
            std::unique_ptr<Connection> c(conn);
            connMap[conn->getFd()] = std::move(c);
        }
    } else if(act == ACTION_DELETE && conn->getInEpoll())
    {
        connMap[conn->getFd()] = nullptr;
        epoll_ctl(epfd, EPOLL_CTL_DEL, conn->getFd(), nullptr);

        // delete conn;
    } 
}

std::vector<Connection*> Epoll::wait(int timeout)
{
    std::vector<Connection*> res;
    int nready = epoll_wait(epfd, events, MAX_EPOLL_CONN, timeout);
    for(int i=0;i<nready;++i)
    {
        auto c = connMap[events[i].data.fd].get();
        // 设置返回的event类型
        c->setREvent(events[i].events);
        res.push_back(c);
    }
    return res;
}

void Epoll::setRecvCallback(Connection* conn, std::function<void(Connection*, Epoll*)> f)
{
    conn->setRecvCallback(std::bind(f, std::forward<Connection*>(conn), std::forward<Epoll*>(this))); 
}

void Epoll::setSendCallback(Connection* conn, std::function<void(Connection*, Epoll*)> f)
{
    conn->setSendCallback(std::bind(f, std::forward<Connection*>(conn), std::forward<Epoll*>(this)));
}

#include "Connection.hpp"
#include "InetAddress.hpp"
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <functional>

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

void Connection::setEvent(uint32_t flags)
{
    event.data.fd = fd;
    event.events = flags;
}

void Connection::setREvent(uint32_t flags)
{
    event.data.fd = fd;
    event.events = flags;
}

epoll_event* Connection::getEvent()
{
    return &event;
}

epoll_event* Connection::getREvent()
{
    return &revent;
}

void Connection::setRecvCallback(std::function<void()> f)
{
    recvCallback = f;
}

void Connection::setSendCallback(std::function<void()> f)
{
    sendCallback = f;
}

void Connection::exeRecvCallback()
{
    recvCallback();
}

void Connection::exeSendCallback()
{
    sendCallback();
}

const char* Connection::getwBuffer()
{
    return wbuffer;
}

const char* Connection::getrBuffer()
{
    return rbuffer;
}

const int Connection::getCurRlen() const {
    return rlen;
}

const int Connection::getCurWlen() const {
    return wlen;
}

void Connection::setRLen(int _rlen)
{
    rlen = _rlen;
}

void Connection::setWLen(int _wlen)
{
    wlen = _wlen;
}
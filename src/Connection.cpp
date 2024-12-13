#include "Connection.hpp"
#include "InetAddress.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <functional>

Connection::Connection()
{
}

Connection::Connection(int _fd, InetAddress *_inetAddr) : fd(_fd), inetAddr(_inetAddr) {}

Connection::Connection(const char *ip, const int port)
{
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
    revent.data.fd = fd;
    revent.events = flags;
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

void Connection::wBufferClear()
{
    memset(wbuffer, 0, SEND_BUFFER_LENGTH);
}

void Connection::rBufferClear()
{
    memset(rbuffer, 0, RECV_BUFFER_LENGTH);
}

char* Connection::getwBuffer()
{
    return wbuffer;
}

char* Connection::getrBuffer()
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
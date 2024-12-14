#include "Socket.hpp"
#include "InetAddress.hpp"
#include "log/util.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>

Socket::Socket()
{
    fd = socket(PF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create failed!");

    InetAddress *_inetAddr = new InetAddress("127.0.0.1", 8080);
    inetAddr = _inetAddr;
    this->bind();
}

Socket::Socket(int _fd, InetAddress *_inetAddr)
{
    errif(fd == -1, "in Socket: fd==-1!");
    fd = _fd;
    inetAddr = _inetAddr;
    this->bind();
}

Socket::Socket(const char* ip, const int port) 
{
    fd = socket(PF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create failed!");
    
    // todo:设置epoll的event

    InetAddress *_inetAddr = new InetAddress(ip, port);
    inetAddr = _inetAddr;
    this->bind();
}

Socket::~Socket() {

}

void Socket::bind()
{
    int res = ::bind(fd, (struct sockaddr*)&inetAddr->addr, inetAddr->addr_sz);
    errif(res == -1, "bind error!");
}

void Socket::listen(int n)
{
    int res = ::listen(fd, n);
    errif(res == -1, "listen error!");
}

Connection* Socket::accept()
{
    // todo: Connection内存池
    InetAddress *_inetAddr = new InetAddress();
    int clientfd = ::accept(fd, (struct sockaddr*)&_inetAddr->addr, &_inetAddr->addr_sz);
    Connection *conn = new Connection(clientfd, _inetAddr);
    return conn;
}
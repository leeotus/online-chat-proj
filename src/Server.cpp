#include "Server.hpp"
#include "Socket.hpp"
#include "EventLoop.hpp"
#include "log/util.hpp"

#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void acceptCallback(Connection* conn ,Epoll* epoll);
void recvCallback(Connection* conn, Epoll *epoll);
void sendCallback(Connection* conn, Epoll *epoll);

Server::Server(EventLoop *_loop)
{
    errif(loop != nullptr, "EventLoop * is nullptr!");
    loop = _loop;
    servSock = new Socket("127.0.0.1", 8080);
    loop->updateConnection(static_cast<Connection*>(servSock), ACTION_UPDATE);

    // 设置服务器端的接收回调函数
    loop->setRecvCallback(static_cast<Connection*>(servSock), acceptCallback);
}


void acceptCallback(Connection* conn, Epoll *epoll)
{
    Socket *servSock = static_cast<Socket*>(conn);
    // 接收来自客户端的请求
    auto client_conn = servSock->accept();
    if(client_conn == nullptr)
    {
        // todo: log
        return;
    }

    // debug:
    printf("new client %d \r\n", client_conn->getFd());

    client_conn->setNonBlocking();
    client_conn->setEvent(EPOLLET | EPOLLIN);
    epoll->updateConnection(client_conn, ACTION_UPDATE);

    epoll->setRecvCallback(client_conn, recvCallback);
    // todo: 设置发送回调函数

}

void recvCallback(Connection *conn, Epoll *epoll)
{
    int recv_bytes{0};
    while(true)
    {
        int rlen = conn->getCurRlen();
        recv_bytes = recv(conn->getFd(), conn->getrBuffer() + rlen, RECV_BUFFER_LENGTH - rlen, 0);
        if(
            recv_bytes == 0 || 
            (recv_bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        ) {
            break;
        }
    }
    // debug:
    printf("recv from client: %s\r\n", conn->getrBuffer());
    // todo: 设置为EPOLLOUT
}
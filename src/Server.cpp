#include "Server.hpp"
#include "Socket.hpp"
#include "EventLoop.hpp"
#include "log/util.hpp"
#include "pools/ThreadPool.hpp"
#include "opengauss/GaussConnector.hpp"
#include "opengauss/libpq-fe.h"

#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void trim(char* str) {
    int start = 0;
    int end = strlen(str) - 1;

    // 找到第一个不是空格或换行符的字符的位置
    while (start <= end && (str[start] == ' ' || str[start] == '\n' || str[start] == '\r' || str[start] == '\t')) {
        start++;
    }

    // 找到最后一个不是空格或换行符的字符的位置
    while (end >= start && (str[end] == ' ' || str[end] == '\n' || str[end] == '\r' || str[end] == '\t')) {
        end--;
    }

    // 如果字符串全是空格或换行符，则将字符串置为空
    if (start > end) {
        str[0] = '\0';
    } else {
        // 将字符串中多余的空格或换行符替换为字符串结束符
        for (int i = start; i <= end; i++) {
            str[i - start] = str[i];
        }
        str[end - start + 1] = '\0';
    }
}

void acceptCallback(Connection* conn ,Epoll* epoll, GaussConnector* dbConnector);
void recvCallback(Connection* conn, Epoll *epoll);
void sendCallback(Connection* conn, Epoll *epoll, GaussConnector *dbConnector);

Server::Server(EventLoop *_loop) : dbConnector(new GaussConnector)
{
    errif(loop != nullptr, "EventLoop * is nullptr!");
    loop = _loop;
    servSock = new Socket("127.0.0.1", 8080);
    servSock->listen(MAX_LISTEN_NUM);
    servSock->setEvent(EPOLLIN|EPOLLET);
    servSock->setNonBlocking();
    loop->updateConnection(static_cast<Connection*>(servSock), ACTION_UPDATE);

    // 设置服务器端的接收回调函数
    loop->setRecvCallback(static_cast<Connection*>(servSock), 
        std::bind(
            acceptCallback, std::placeholders::_1, std::placeholders::_2, this->getDBConnector()
        )
    );
}

Server::~Server()
{
    delete servSock;
    delete loop;
}

GaussConnector* Server::getDBConnector()
{
    return dbConnector.get();
}


void acceptCallback(Connection* conn, Epoll *epoll, GaussConnector *dbConnector)
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
    epoll->setSendCallback(client_conn, 
            std::bind(
                sendCallback, 
                std::placeholders::_1, 
                std::placeholders::_2, 
                dbConnector
            ));
}

void recvCallback(Connection *conn, Epoll *epoll)
{
    int recv_bytes{0};
    // 清除缓存
    conn->rBufferClear();
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
        conn->setRLen(rlen + recv_bytes);
    }
    // debug:
    printf("recv from client: %s\r\n", conn->getrBuffer());

    if(strlen(conn->getrBuffer()) != 0)
    {
        conn->setEvent(EPOLLOUT | EPOLLET);
        epoll->updateConnection(conn, ACTION_UPDATE);
    } else {
        // 如果客户端啥都没有发来:
        epoll->updateConnection(conn, ACTION_DELETE);
    }
}

void sendCallback(Connection *conn, Epoll *epoll, GaussConnector *dbConnector)
{
    // debug:
    char buffer[1024];
    trim(conn->getrBuffer());
    strcpy(buffer, conn->getrBuffer());
    if(strcmp(buffer, "LogIn"))
    {
        // 登录操作:
        const char delims[] = " ?";
        strtok(buffer, delims);
        char *username = strtok(NULL, delims);
        char *password = strtok(NULL, delims);
        printf("username = %s,pwd=%s\r\n", username, password);

        // todo: 查找操作:
        char query[2048];
        char errmsgBuffer[1024];
        sprintf(query, "select * from user_info where username = '%s' and password = '%s';", username, password);
        //debug:
        printf("query = %s\r\n", query);
        int res = dbConnector->searchForOne(query, errmsgBuffer);
        if(res == 0)
        {
            strcpy(conn->getwBuffer(), "Pass");
            conn->setWLen(strlen("Pass"));
        } else {
            strcpy(conn->getwBuffer(), errmsgBuffer);
            conn->setWLen(strlen(errmsgBuffer));
        }
    }

    send(conn->getFd(), conn->getwBuffer(), conn->getCurWlen(), 0);

    // close:
    epoll->updateConnection(conn, ACTION_DELETE);
}
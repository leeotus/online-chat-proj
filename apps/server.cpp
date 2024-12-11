#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "log/util.hpp"
#include "opengauss/GaussConnector.hpp"

#define MAX_LISTEN_NUM 15
#define BUFFER_LENGTH 1024
#define MAX_EPOLL_CONNECTION 16192

void setNonBlocking(int fd);

int main(int argc, char **argv)
{
    GaussConnector connector;

    char buffer[BUFFER_LENGTH];
    int bufferLen = 0;
    const char *ip = "127.0.0.1";
    const int port = 8080;
    int epfd;
    struct epoll_event events[MAX_EPOLL_CONNECTION];
    struct epoll_event ev;

    int serverfd = socket(PF_INET, SOCK_STREAM, 0);
    errif(serverfd == -1, "socket create failed!");

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(port);

    // 创建epoll红黑树
    epfd = epoll_create(1);
    errif(epfd == -1, "epoll create failed!");

    errif((bind(serverfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1), "bind error!");
    errif((listen(serverfd, MAX_LISTEN_NUM) == -1), "listen error!");

    ev.data.fd = serverfd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serverfd, &ev);

    while(true)
    {
        int nready = epoll_wait(epfd, events, MAX_EPOLL_CONNECTION, -1);
        for(int i=0;i<nready;++i)
        {
            int fd = events[i].data.fd;
            if(fd == serverfd)
            {
                // 有客户端发来请求
                int clientfd = accept(serverfd, NULL, NULL);
                printf("new client:%d\r\n", clientfd);
                if(clientfd == -1)
                {
                    continue;
                }
                setNonBlocking(clientfd);
                ev.data.fd = clientfd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
            } else if(events[i].events == EPOLLIN)
            {
                bzero(&buffer, BUFFER_LENGTH);
                bufferLen = 0;

                int recv_bytes;
                while(true)
                {
                    recv_bytes = recv(fd, buffer+bufferLen, BUFFER_LENGTH-bufferLen, 0);
                    if(
                        recv_bytes == 0 || 
                        (recv_bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    ){
                        break;
                    }
                    bufferLen += recv_bytes;
                }
                //debug:
                printf("recv from client: %s\r\n", buffer);

                // 测试，直接断开:
                close(fd);
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
            } else if(events[i].events == EPOLLOUT)
            {
                // TODO
            }
        }
    }

    epoll_ctl(epfd, EPOLL_CTL_DEL, serverfd, nullptr);
    close(serverfd);

    return 0;
}

void setNonBlocking(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}
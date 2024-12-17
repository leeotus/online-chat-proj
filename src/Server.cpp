#include "Server.hpp"
#include "Socket.hpp"
#include "EventLoop.hpp"
#include "log/util.hpp"
#include "opengauss/tables/UserInfo.hpp"
#include "pools/ThreadPool.hpp"
#include "opengauss/GaussConnector.hpp"
#include "opengauss/libpq-fe.h"

#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <vector>

#define BUFFER_LENGTH 1024
#define QUERY_BUFFER_SIZE 2048

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
    servSock = new Socket("10.0.12.2", 8080);
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
    conn->wBufferClear();
    char buffer[BUFFER_LENGTH];
    trim(conn->getrBuffer());

    // 获取客户端输入的所有信息:
    strcpy(buffer, conn->getrBuffer());
    if(strstr(buffer, "LogIn"))
    {
        // 请求一:用户请求登录
        UserInfo userinfo;
        char errmsgBuffer[BUFFER_LENGTH];
        memset(errmsgBuffer, 0, BUFFER_LENGTH);

        // 登录操作:
        const char delims[] = " ?";
        strtok(buffer, delims);

        // 获取用户名字和用户密码
        char *userId = strtok(NULL, delims);
        char *password = strtok(NULL, delims);

        char queryCmd[QUERY_BUFFER_SIZE];
        sprintf(queryCmd, "userid = '%s'", userId);
        auto searchRes = userinfo.search(queryCmd, errmsgBuffer);

        if(searchRes.size() == 0)
        {
            // err1.找不到该用户
            sprintf(conn->getwBuffer(), "%s", "NoUid");
            conn->setWLen(strlen("NoUid"));
            send(conn->getFd(), conn->getwBuffer(), conn->getCurWlen(), 0);
            epoll->updateConnection(conn, ACTION_DELETE);
            return;
        } else {
            // 查看密码是否正确:
            sprintf(queryCmd, "userid = '%s' and password = '%s'", userId, password);
            auto searchRes = userinfo.search(queryCmd, errmsgBuffer);
            if(!searchRes.empty())
            {
                // 设置用户为登录状态:
                sprintf(queryCmd, "update user_info set online = 1 where userid = '%s'", userId);
                userinfo.modify(queryCmd, errmsgBuffer);
                sprintf(conn->getwBuffer(), "%s %s", "Pass", searchRes[0][2].c_str());
                conn->setWLen(strlen(conn->getwBuffer()));
            } else {
                sprintf(conn->getwBuffer(), "%s", "WrongPwd");
                conn->setWLen(strlen("WrongPwd"));
                send(conn->getFd(), conn->getwBuffer(), conn->getCurWlen(), 0);
                epoll->updateConnection(conn, ACTION_DELETE);
                return;
            }
        }
    }
    else if(strstr(buffer, "ChangePwd"))
    {
        // 修改密码:
        UserInfo userinfo;
        char errmsgBuffer[BUFFER_LENGTH];
        memset(errmsgBuffer, 0, BUFFER_LENGTH);

        const char delims[] = " ?";
        strtok(buffer, delims);

        // 获取uid,旧密码和新密码
        char *userId = strtok(NULL, delims);
        char *oldpwd = strtok(NULL, delims);
        char *newpwd = strtok(NULL, delims);

        char queryCmd[QUERY_BUFFER_SIZE];
        sprintf(queryCmd, "userid = '%s'", userId);
        auto searchRes = userinfo.search(queryCmd, errmsgBuffer);

        if(searchRes.size() == 0)
        {
            // 说明没有这个账号:
            sprintf(conn->getwBuffer(), "%s", "NoUid");
            conn->setWLen(strlen("NoUid"));
            send(conn->getFd(), conn->getwBuffer(), conn->getCurWlen(), 0);
            epoll->updateConnection(conn, ACTION_DELETE);
            return;
        } else {
            if(!strcmp(searchRes[0][1].c_str(), oldpwd))
            {
                // 修改密码:
                sprintf(queryCmd, "update user_info set password = '%s' where userid = '%s'", newpwd, userId); 
                userinfo.modify(queryCmd, errmsgBuffer);
                sprintf(conn->getwBuffer(), "%s", "Pass");
                conn->setWLen(strlen(conn->getwBuffer()));
                // 修改密码不需要长连接
                send(conn->getFd(), conn->getwBuffer(), conn->getCurWlen(), 0);
                epoll->updateConnection(conn, ACTION_DELETE);
                return;
            } else {
                // 原密码错误:
                sprintf(conn->getwBuffer(), "%s", "WrongPwd");
                conn->setWLen(strlen("WrongPwd"));
                send(conn->getFd(), conn->getwBuffer(), conn->getCurWlen(), 0);
                epoll->updateConnection(conn, ACTION_DELETE);
                return;
            }
        }
    }
    else if(strstr(buffer, "SignUp"))
    {
        // 注册
        UserInfo userinfo;
        char errmsgBuffer[BUFFER_LENGTH];
        memset(errmsgBuffer, 0, BUFFER_LENGTH);

        const char delims[] = " ?";
        strtok(buffer, delims);

        // 获取用户id,密码和用户名
        char *userId = strtok(NULL, delims);
        char *password = strtok(NULL, delims);
        char *username = strtok(NULL, delims);

        char queryCmd[QUERY_BUFFER_SIZE];
        // 先查询账号是否存在:
        sprintf(queryCmd, "userid = '%s'", userId);
        auto searchRes = userinfo.search(queryCmd, errmsgBuffer);
        if(searchRes.size() != 0)
        {
            // 说明已经有该账号了:
            sprintf(conn->getwBuffer(), "%s", "UidExist");
            conn->setWLen(strlen("UidExist"));
            send(conn->getFd(), conn->getwBuffer(), conn->getCurWlen(), 0);
            epoll->updateConnection(conn, ACTION_DELETE);
            return;
        }
        // 如果账号不存在,注册一个:
        sprintf(queryCmd, "insert into user_info values('%s', '%s', '%s', 0, '')", userId, password, username);
        int res = userinfo.insert(queryCmd, errmsgBuffer);
        if(res == 0)
        {
            // 注册成功:
            sprintf(conn->getwBuffer(), "%s", "Pass");
            conn->setWLen(strlen("Pass"));
        } else {
            // for debug:
            sprintf(conn->getwBuffer(), "%s", "Failed to signup!!!");
            conn->setWLen(strlen(conn->getwBuffer()));
        }
        // 注册不需要长连接
        send(conn->getFd(), conn->getwBuffer(), conn->getCurWlen(), 0);
        epoll->updateConnection(conn, ACTION_DELETE);
        return;
    }
    else if(strstr(buffer, "LogOut"))
    {
        // 退出操作:
        UserInfo userinfo;
        char errmsgBuffer[BUFFER_LENGTH];
        memset(errmsgBuffer, 0, BUFFER_LENGTH);

        const char delims[] = " ?";
        strtok(buffer, delims);

        // 获取用户id
        char *userId = strtok(NULL, delims);
        char queryCmd[QUERY_BUFFER_SIZE];
        sprintf(queryCmd, "update user_info set online = 0 where userid = '%s'", userId);
        // 修改登录信息:
        userinfo.modify(queryCmd, errmsgBuffer);

        // 从epoll红黑树删除:
        epoll->updateConnection(conn, ACTION_DELETE);
        return;
    }
    else if(strstr(buffer, "Send"))
    {
        UserInfo userinfo;
        char errmsgBuffer[BUFFER_LENGTH];
        memset(errmsgBuffer, 0, BUFFER_LENGTH);

        // const char delims[] = " ?";
        // strtok(buffer, delims);

        // 获取用户发上来的消息,之后需要发送给其他用户
        // char *message = strtok(NULL, delims);
        // epoll->sendMsg2AllExcept(conn->getFd(), message);
        epoll->sendMsg2AllExcept(conn->getFd(), buffer);
        conn->setEvent(EPOLLIN | EPOLLET);
        epoll->updateConnection(conn, ACTION_UPDATE);
        return;
    }
    else if(strstr(buffer, "FollowingOnline"))
    {
        UserInfo userinfo;
        char errmsgBuffer[BUFFER_LENGTH];
        memset(errmsgBuffer, 0, BUFFER_LENGTH);

        const char delims[] = " ?";
        strtok(buffer, delims);

        // 获取用户id
        char *userId = strtok(NULL, delims);

        char queryCmd[QUERY_BUFFER_SIZE];
        sprintf(queryCmd, "uid = '%s'", userId);
        auto searchRes = userinfo.search(queryCmd, errmsgBuffer, "following");

        strcpy(conn->getwBuffer(), "FollowingOnline ");
        if(!searchRes.empty())
        {
            for(auto &relation : searchRes)
            {
                char followId[BUFFER_LENGTH];
                char followName[BUFFER_LENGTH];
                strcpy(followId, relation[1].c_str());
                // 查询关注的用户其昵称和是否在线
                sprintf(queryCmd, "userid = '%s'", followId);

                auto userRes = userinfo.search(queryCmd, errmsgBuffer);
                if(userRes.empty())
                {
                    continue;
                }
                if(strstr(userRes[0][3].c_str(), "1"))
                {
                    // 说明关注的用户在线:
                    sprintf(followName, "#%s ", userRes[0][2].c_str());
                    strcat(conn->getwBuffer(), followId);
                    strcat(conn->getwBuffer(), followName);
                }
            }
        } else {
            strcat(conn->getwBuffer(), "Empty");
        }
        strcat(conn->getwBuffer(), "%");
        conn->setWLen(strlen(conn->getwBuffer()));
    }
    else if(strstr(buffer, "Follow"))
    {
        UserInfo userinfo;
        char errmsgBuffer[BUFFER_LENGTH];
        memset(errmsgBuffer, 0, BUFFER_LENGTH);

        const char delims[] = " ?";
        strtok(buffer, delims);

        // 获取用户的id和要关注的id:
        char *userId = strtok(NULL, delims);
        char *want2followId = strtok(NULL, delims);

        char queryCmd[QUERY_BUFFER_SIZE];
        sprintf(queryCmd, "insert into following values('%s', '%s')", userId, want2followId);
        int res = userinfo.insert(queryCmd, errmsgBuffer);
        if(res != 0)
        {
            sprintf(conn->getwBuffer(), "%s", "NoUid %");
            conn->setWLen(strlen(conn->getwBuffer()));
        } else {
            // 不用发送
            conn->setEvent(EPOLLIN | EPOLLET);
            epoll->updateConnection(conn, ACTION_UPDATE);
            return;
        }
    } 
    else if(strstr(buffer, "Unfollow"))
    {
        UserInfo userinfo;
        char errmsgBuffer[BUFFER_LENGTH];
        memset(errmsgBuffer, 0, BUFFER_LENGTH);

        const char delims[] = " ?";
        strtok(buffer, delims);

        // 获取用户id和要取消关注的id
        char *userId = strtok(NULL, delims);
        char *delId = strtok(NULL, delims);

        char queryCmd[QUERY_BUFFER_SIZE];
        sprintf(queryCmd, "delete from following where uid = '%s' and following_id = '%s'", userId, delId);
        userinfo.modify(queryCmd, errmsgBuffer);
        conn->setEvent(EPOLLIN | EPOLLET);
        epoll->updateConnection(conn, ACTION_UPDATE);
        return;
    }

    send(conn->getFd(), conn->getwBuffer(), conn->getCurWlen(), 0);

    // todo:需要保持长连接
    conn->setEvent(EPOLLIN | EPOLLET);
    epoll->updateConnection(conn, ACTION_UPDATE);
}
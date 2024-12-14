#pragma once
#include <memory>

class Socket;
class EventLoop;
class Connection;
class Epoll;
class GaussConnector;

class Server 
{
public:
    Server(EventLoop *_loop);
    ~Server();

    GaussConnector* getDBConnector();

private:
    Socket *servSock;
    EventLoop *loop;

    // opengauss数据库连接
    std::unique_ptr<GaussConnector> dbConnector;
};
#pragma once

class Socket;
class EventLoop;
class Connection;
class Epoll;

class Server 
{
public:
    Server(EventLoop *_loop);
    ~Server();

private:
    Socket *servSock;
    EventLoop *loop;
};
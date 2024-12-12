#include "Server.hpp"
#include "EventLoop.hpp"

int main(int argc, char **argv)
{
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);
    loop->start();
}
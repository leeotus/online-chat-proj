#pragma once
#include <map>
#include <sys/epoll.h>
#include <vector>

#define MAX_EPOLL_CONN 16384

typedef enum __EPOLL_ACTION {
    ACTION_UPDATE,
    ACTION_DELETE,
}EpollAction;

class Connection;
class Epoll 
{
public:
    Epoll();

    ~Epoll();

    void updateConnection(Connection* conn, EpollAction act);

    /**
     * @brief 等待有新文件在epoll红黑树中被激活
     * @param timeout 等待时间
     * @return std::vector<Connection*> 
     */
    std::vector<Connection*> wait(int timeout=-1);

    
private:
    int epfd;
    std::map<int, Connection*> connMap;
    struct epoll_event events;
};
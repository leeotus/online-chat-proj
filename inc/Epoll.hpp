#pragma once
#include <map>
#include <sys/epoll.h>
#include <vector>
#include <functional>

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

    /**
     * @brief 更新连接操作
     * @param conn 要更新的Connection对象 
     * @param act 更新操作类型
     * @note 如果要执行的是ACTION_UPDATE,那么当conn对象不在epoll红黑树的时候
     * 会将其添加到epoll红黑树中,反之如果conn已经在epoll红黑树中则执行modify修改
     * 如果要执行的是ACTION_DELETE,该函数不会判断conn是否已经在epoll红黑树中
     * 如果不在epoll红黑树中则该函数没有效果.
     */
    void updateConnection(Connection* conn, EpollAction act);

    /**
     * @brief 等待有新文件在epoll红黑树中被激活
     * @param timeout 等待时间
     * @return std::vector<Connection*> 
     */
    std::vector<Connection*> wait(int timeout=-1);

    /**
     * @brief Set the Recv Callback object
     * 设置Connection对象的recv回调函数
     * @param conn 要设置recv回调函数的Connection连接对象
     * @param f 回调函数
     */
    void setRecvCallback(Connection* conn, std::function<void(Connection*, Epoll*)> f);

    void setSendCallback(Connection* conn, std::function<void(Connection*, Epoll*)> f);
    
private:
    int epfd;
    std::map<int, Connection*> connMap;
    struct epoll_event events[MAX_EPOLL_CONN];
};
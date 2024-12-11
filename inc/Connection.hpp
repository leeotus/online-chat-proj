#pragma once

#include <sys/epoll.h>

class InetAddress;
class Connection {
public:
    Connection();
    Connection(int _fd, InetAddress *_inetAddr);
    Connection(const char* ip, const int port);
    ~Connection();

    /**
     * @brief Get the Fd object
     * 获取保存的文件描述符
     * @return int 返回保存的文件描述符
     */
    inline int getFd() const
    {
        return fd;
    }

    /**
     * @brief Get the Inet Address object
     * 获取套接字的套接字地址
     * @return const InetAddress* 
     * @note 直接返回,不会判断是否是null
     */
    inline const InetAddress* getInetAddress()
    {
        return inetAddr;
    }

    inline void setInetAddr(InetAddress *_inetAddr)
    {
        inetAddr = _inetAddr; 
    }

    /**
     * @brief Set the Non Blocking object
     * 设置文件描述符为非阻塞模式
     */
    void setNonBlocking();

    inline void setInEpoll()
    {
        inEpoll = true;
    }

protected:
    int fd{-1};     // 文件描述符
    bool inEpoll{false};       // 是否已经被保存在epoll红黑树中
    InetAddress* inetAddr{nullptr};      // 套接字地址指针

private:
    struct epoll_event event;
    struct epoll_event revent;
};
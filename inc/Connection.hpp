#pragma once

#include <sys/epoll.h>
#include <functional>

#define RECV_BUFFER_LENGTH 2048
#define SEND_BUFFER_LENGTH 2048

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
     * @brief 获取当前的连接是否已经被放入到epoll红黑树中
     * @return true 
     * @return false 
     */
    inline bool getInEpoll() const
    {
        return inEpoll;
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

    /**
     * @brief Set the Non Blocking object
     * 设置文件描述符为非阻塞模式
     */
    void setNonBlocking();

    inline void setInEpoll()
    {
        inEpoll = true;
    }

    void setEvent(uint32_t flags);
    void setREvent(uint32_t flags);

    epoll_event* getEvent();
    epoll_event* getREvent();

    /**
     * @brief 执行recv回调函数
     */
    void exeRecvCallback();

    /**
     * @brief 执行send回调函数
     */
    void exeSendCallback();

    void setRecvCallback(std::function<void()> _recvCallback);
    void setSendCallback(std::function<void()> _sendCallback);

    const char* getwBuffer();
    const char* getrBuffer();
    const int getCurWlen() const;
    const int getCurRlen() const;
    void setRLen(int _rlen);
    void setWLen(int _wlen);

protected:
    int fd{-1};     // 文件描述符
    bool inEpoll{false};       // 是否已经被保存在epoll红黑树中
    InetAddress* inetAddr{nullptr};      // 套接字地址指针

private:
    struct epoll_event event;
    struct epoll_event revent;

    char rbuffer[RECV_BUFFER_LENGTH];
    int  rlen{0};
    char wbuffer[SEND_BUFFER_LENGTH];
    int wlen{0};

    std::function<void()> recvCallback;
    std::function<void()> sendCallback;
};
#include <cerrno>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define BUFFER_LENGTH 1024

void errif(bool condition, const char *msg);
void setNonBlocking(int fd);

int main(int argc, char **argv)
{
    int clientfd;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_LENGTH];

    clientfd = socket(PF_INET, SOCK_STREAM, 0);
    errif(clientfd == -1, "socket create failed !");
    // setNonBlocking(clientfd);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    socklen_t socklen = sizeof(serverAddr);
    int res = connect(clientfd, (struct sockaddr*)&serverAddr, socklen);
    errif(res == -1, "connect error!");

    while(true)
    {
        int rlen = 0;
        fgets(buffer, BUFFER_LENGTH, stdin);
        send(clientfd, buffer, strlen(buffer), 0);
        bzero(buffer, BUFFER_LENGTH);
        while(true)
        {
            int read_bytes = recv(clientfd, buffer+rlen, BUFFER_LENGTH-rlen, 0);
            if(
                read_bytes == 0 || 
                (read_bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            )
            {
                break;
            }
            rlen += read_bytes;
        }
        printf("recv from server: %s\r\n", buffer);
    }
    close(clientfd);

    return 0;
}


void errif(bool condition, const char *msg)
{
    if(condition)
    {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

void setNonBlocking(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);    
}
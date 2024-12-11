#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_LENGTH 1024

void errif(bool condition, const char *msg);

int main(int argc, char **argv)
{
    int clientfd;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_LENGTH];

    clientfd = socket(PF_INET, SOCK_STREAM, 0);
    errif(clientfd == -1, "socket create failed !");

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    socklen_t socklen = sizeof(serverAddr);
    int res = connect(clientfd, (struct sockaddr*)&serverAddr, socklen);
    errif(res == -1, "connect error!");

    while(true)
    {
        fgets(buffer, BUFFER_LENGTH, stdin);
        send(clientfd, buffer, strlen(buffer), 0);
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
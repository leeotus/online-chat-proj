#include <string.h>
#include "opengauss/GaussConnector.hpp"
#include "opengauss/libpq-fe.h"

#define QUERY_BUFFER_SIZE 1024

GaussConnector::GaussConnector()
{
    conninfo = "dbname=postgres port=5432 host='127.0.0.1' application_name=online-chat connect_timeout=10 sslmode=allow user='superuser' password='OGSql@123'";
    conn = PQconnectdb(conninfo);

    if(PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed:%s\r\n", PQerrorMessage(conn));
        // 错误退出
        exit(EXIT_FAILURE);
    }
    // 创建本项目的表:(还是自己用db软件创建吧，比较方便)
    // res = PQexec(conn, 
    // "CREATE TABLE chat_customers(user_id varchar(255) primary key, user_name varchar(255))"
    // );
    // if(PQresetStart(conn) != PGRES_COMMAND_OK)
    // {
    //     printf("table create failed or has already exit!\r\n");
    // }
}

GaussConnector::~GaussConnector()
{
    PQfinish(conn);
}

int GaussConnector::insert(const char* cmd, char *errmsgBuffer)
{
    res = PQexec(conn, cmd);
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        if(errmsgBuffer != nullptr)
        {
            strcat(errmsgBuffer, PQerrorMessage(conn));
        }
        PQclear(res);
        return -1;
    }
    return 0;
}

int GaussConnector::update(const char* cmd, char *errmsgBuffer)
{
    res = PQexec(conn, cmd);
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        if(errmsgBuffer != nullptr)
        {
            strcat(errmsgBuffer, PQerrorMessage(conn));
        }
        PQclear(res);
        return -1;
    }
    return 0;
}

int GaussConnector::omit(const char *cmd, char *errmsgBuffer)
{
    res = PQexec(conn, cmd);
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        if(errmsgBuffer != nullptr)
        {
            strcat(errmsgBuffer, PQerrorMessage(conn));
        }
        PQclear(res);
        return -1;
    }
    return 0;
}
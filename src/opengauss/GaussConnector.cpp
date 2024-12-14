#include <string.h>
#include "opengauss/GaussConnector.hpp"
#include "opengauss/libpq-fe.h"

#define QUERY_BUFFER_SIZE 1024

GaussConnector::GaussConnector()
{
    // note: 159.75.88.12此服务器将于1月份过期
    conninfo = "dbname=postgres port=5432 host='159.75.88.12' application_name=online-chat connect_timeout=10 sslmode=allow user='superuser' password='OGSql@123'";
    conn = PQconnectdb(conninfo);

    if(PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed:%s\r\n", PQerrorMessage(conn));
        // 错误退出
        exit(EXIT_FAILURE);
    }
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
#include <string.h>
#include <tuple>
#include <vector>
#include <stdlib.h>
#include <functional>
#include <unistd.h>
#include <string>

#include "opengauss/libpq-fe.h"
#include "opengauss/tables/UserInfo.hpp"
#include "opengauss/GaussConnector.hpp"
#include "log/util.hpp"

#define DATA_LENGTH 1024
#define TRANSACTION_LENGTH 128
#define QUERY_LENGTH 512
#define TOTAL_QUERY_LENGTH 1024

UserInfo::UserInfo()
{
    strcpy(tableName, "user_info");    
}

UserInfo::~UserInfo()
{

}

int UserInfo::insert(const char *cmd, char* errmsgBuffer)
{
    PGresult *res;
    PGconn *conn;

    // 连接数据库
    conn = PQconnectdb(conninfo);
    errif(PQstatus(conn)!=CONNECTION_OK, PQerrorMessage(conn));
    res = PQexec(conn, cmd);
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        if(errmsgBuffer!=nullptr)
        {
            strcpy(errmsgBuffer, PQerrorMessage(conn));
        }
        PQclear(res);
        PQfinish(conn);
        return -1;
    }
    PQclear(res);
    PQfinish(conn);
    return 0;
}

int UserInfo::modify(const char *cmd, char *errmsgBuffer)
{
    PGresult *res;
    PGconn *conn;
    // 连接数据库
    conn = PQconnectdb(conninfo);

    errif(PQstatus(conn) != CONNECTION_OK, PQerrorMessage(conn));
    res = PQexec(conn, cmd);
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        if(errmsgBuffer!=nullptr)
        {
            strcpy(errmsgBuffer, PQerrorMessage(conn));
        }
        PQclear(res);
        PQfinish(conn);
        return -1;
    }
    PQclear(res);
    PQfinish(conn);
    return 0;
}

std::vector<std::vector<std::string>> UserInfo::search(const char* condition, char *errmsgBuffer, const char *tableName)
{
    int nFields;
    PGconn *conn;
    conn = PQconnectdb(conninfo);

    // 如果数据库连接出错
    errif(PQstatus(conn) != CONNECTION_OK, PQerrorMessage(conn));
    
    std::hash<const char*> char_hash;
    char transactionName[TRANSACTION_LENGTH];
    char cmd[TOTAL_QUERY_LENGTH];
    char query[QUERY_LENGTH];

    memset(transactionName, 0, TRANSACTION_LENGTH);
    memset(cmd, 0, TOTAL_QUERY_LENGTH);
    memset(query, 0, QUERY_LENGTH);

    // 查询语句
    if(strlen(condition) != 0)
    {
        sprintf(query, "select * from %s where %s", tableName, condition);
    } else {
        sprintf(query, "select * from %s", tableName);
    }
    // 设定事务名称
    sprintf(transactionName, "Search%zu", char_hash(query) % 1000000);
    // 要进行处理的事务语句:
    sprintf(cmd, "DECLARE %s CURSOR FOR %s", transactionName, query);
    //debug: 打印要处理的事务语句
    // printf("query: %s\r\n", cmd);

    std::vector<std::vector<std::string>> lines{};

    // 开始一个事务块
    PGresult *res;
    res = PQexec(conn, "BEGIN");
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        if(errmsgBuffer!=nullptr)
        {
            strcpy(errmsgBuffer, PQerrorMessage(conn));
        }
        PQclear(res);
        PQfinish(conn);
        return lines;
    }

    // 在结果不需要的时候PQclear PGresult,以避免内存泄露
    PQclear(res);

    // 从'user_info'表中抓取数据
    res = PQexec(conn, cmd);
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        if(errmsgBuffer!=nullptr)
        {
            strcpy(errmsgBuffer, PQerrorMessage(conn));
        }
        PQclear(res);
        PQfinish(conn);
        return lines;
    }
    PQclear(res);

    bzero(cmd, TOTAL_QUERY_LENGTH);
    // 设置命令
    sprintf(cmd, "FETCH ALL in %s", transactionName);

    res = PQexec(conn, cmd);
    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        if(errmsgBuffer!=nullptr)
        {
            strcpy(errmsgBuffer, PQerrorMessage(conn));
        }
        PQclear(res);
        PQfinish(conn);
        return lines;
    }

    // 获取属性列数
    nFields = PQnfields(res);
    for(int i = 0; i < PQntuples(res); ++i)
    {
        std::vector<std::string> item;
        for(int j = 0; j < nFields; ++j)
        {
            const char* value = PQgetvalue(res, i, j);
            item.push_back(value ? value : "");
        }
        lines.push_back(item);
    }


    PQclear(res);

    // 关闭入口:
    bzero(cmd, TOTAL_QUERY_LENGTH);
    sprintf(cmd, "CLOSE %s", transactionName);
    res = PQexec(conn, cmd);
    PQclear(res);

    // 结束事务
    res = PQexec(conn, "END");
    PQclear(res);
    PQfinish(conn);

    return lines;
}
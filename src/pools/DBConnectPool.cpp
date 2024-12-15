#include "pools/DBConnectPool.hpp"
#include <stdlib.h>
#include <unistd.h>
#include "opengauss/libpq-fe.h"
#include "log/util.hpp"

DBConnectPool::DBConnectPool(int _connNum) : connNum(_connNum)
{
    for(int i=0;i<connNum;++i)
    {
        // 连接数据库并将数据库指针放到连接池中
        PGconn *conn = PQconnectdb(conninfo);

        // 如果数据库连接出问题:
        errif(PQstatus(conn) != CONNECTION_OK, PQerrorMessage(conn));
        dbConnectors.push_back(conn);

    }
}
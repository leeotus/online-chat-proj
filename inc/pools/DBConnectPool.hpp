#pragma once

#include <stdlib.h>
#include <unistd.h>
#include "../opengauss/libpq-fe.h"

#include <vector>
#include <mutex>
#include <queue>
#include <memory>
#include <functional>

class DBConnectPool{
    const char *conninfo = "dbname=postgres port=5432 host='159.75.88.12' application_name=online-chat connect_timeout=10 sslmode=allow user='superuser' password='OGSql@123'";
public:
    explicit DBConnectPool(int _connNum);
    ~DBConnectPool();
private:
    std::mutex dbmtx;
    bool quit{false};           // 数据库连接池退出
    int connNum;                // 数据库连接数量
    std::vector<PGconn*> dbConnectors;      // 连接数据库
    std::vector<std::mutex> connectorMtx;
};
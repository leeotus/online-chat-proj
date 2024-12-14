#pragma once

#include <stdlib.h>
#include <stdarg.h>
#include "libpq-fe.h"

#include <string>

class GaussConnector
{
public:
    GaussConnector();
    ~GaussConnector();

    /**
     * @brief 数据库插入操作,向表中插入数据
     * @param cmd 插入的指令
     * @param errmsgBuffer 操作失败的时候如果该参数有指向一个已经分配好的char数组,则会将错误
     * 信息保存在该char数组中,以供查询错误信息
     * @return int 成功插入返回0,失败返回-1
     */
    int insert(const char *cmd, char *errmsgBuffer=nullptr);

    /**
     * @brief 数据库更新操作，更新表中的数据
     * @param cmd 更新指令
     * @param errmsgBuffer 操作失败的时候如果该参数有指向一个已经分配好的char数组,则会将错误
     * 信息保存在该char数组中,以供查询错误信息
     * @return int 成功返回0,失败返回-1
     */
    int update(const char *cmd, char *errmsgBuffer=nullptr);

    /**
     * @brief 数据库删除操作,删除表中的数据
     * @param cmd 删除指令
     * @param errmsgBuffer 操作失败的时候如果该参数有指向一个已经分配好的char数组,则会将错误
     * 信息保存在该char数组中,以供查询错误信息
     * @return int 成功返回0,失败返回-1
     */
    int remove(const char *cmd, char *errmsgBuffer=nullptr);

    int searchForOne(const char* cmd, char *errmsgBuffer=nullptr);

private:
    const char *conninfo;
    PGconn *conn;
    PGresult *res;
    int nFields;
};

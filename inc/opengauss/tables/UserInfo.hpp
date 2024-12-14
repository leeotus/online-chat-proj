#pragma once

#include "Table.hpp"
#include <stdarg.h>

#include <vector>

#define TABLE_NAME_LEN 128
#define USER_INFO_ID 512
#define USER_INFO_PWD 512
#define USER_INFO_NAME 1024
#define USER_INFO_IMG 1024

// 每一行数据
typedef struct __USER_INFO_LINE
{
    char uid[USER_INFO_ID];             // primary key:用户id
    //todo: 对密码使用加密  
    char password[USER_INFO_PWD];       // 用户密码
    char name[USER_INFO_NAME];          // 用户名称
    char imgPath[USER_INFO_IMG];        // 用户头像路径
    bool online;                        // 显示其是否在线
}UserInfoLine;

class GaussConnector;

// 用户信息表
class UserInfo {
public:
    UserInfo();
    ~UserInfo();
    // 增删查改
    int insert(GaussConnector *dbConnector, UserInfoLine* info);

    int remove(GaussConnector *dbConnector, UserInfoLine* info);

    std::vector<UserInfoLine> search();
private:
    char tableName[TABLE_NAME_LEN];
};
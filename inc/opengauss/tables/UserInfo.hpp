#pragma once

#include "Table.hpp"
#include <stdarg.h>

#include <vector>
#include <string>

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
    int insert(const char *cmd, char *errmsgBuffer=nullptr);

    /**
     * @brief 查询操作 
     * @param info 要查询的信息 
     * @param errmsgBuffer 如果查询失败的话会有错误信息保存在此数组中 
     * @return std::vector<UserInfoLine> 返回找到的信息
     */
    std::vector<std::vector<std::string>> search(const char* condition, char* errmsgBuffer = nullptr, const char *tableName = "user_info");

    /**
     * @brief 更新操作
     * @param cmd 修改操作 
     * @param errmsgBuffer 错误信息存储
     * @return int 成功返回0,失败返回-1
     */
    int modify(const char *cmd, char *errmsgBuffer = nullptr);

private:
    char tableName[TABLE_NAME_LEN];
};
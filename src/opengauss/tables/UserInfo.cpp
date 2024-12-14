#include <string.h>

#include "opengauss/tables/UserInfo.hpp"
#include "opengauss/GaussConnector.hpp"

// table name: "user_info"
#define QUERY_LENGTH 2048

UserInfo::UserInfo()
{
    strcpy(tableName, "user_info");    
}

int UserInfo::insert(GaussConnector *dbConnector, UserInfoLine *info)
{
    char query[QUERY_LENGTH];
    return 0;
}
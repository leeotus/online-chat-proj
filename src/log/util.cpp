#include "log/util.hpp"

void errif(bool condition, const char *msg)
{
    if(condition)
    {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}
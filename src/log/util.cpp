#include "log/util.hpp"
#include <stdlib.h>
#include <unistd.h>
#include "opengauss/GaussConnector.hpp"

void errif(bool condition, const char *msg)
{
    if(condition)
    {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}
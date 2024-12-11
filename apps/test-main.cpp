/*
 * testlibpq.c
 */
#include <stdio.h>
#include <stdlib.h>
#include "opengauss/libpq-fe.h"

#include "opengauss/GaussConnector.hpp"

static void
exit_nicely(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

int
main(int argc, char **argv)
{
    GaussConnector gauss;
    gauss.insert("insert into customers values(24,'li')");
}

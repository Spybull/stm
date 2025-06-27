#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "error.h"

int
libstm_init(libstm_error_t *err)
{
    int rc;

    const char *filename = "stm.db";
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    sqlite3 *ppdb;

    rc = sqlite3_open_v2(filename, &ppdb, flags, NULL);
    if (rc != SQLITE_OK)
        return stm_make_error(err, errno, "failed to open/create databse");
    return 0;
}
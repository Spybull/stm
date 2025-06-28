#include <stdio.h>
#include <stdlib.h>

#include "db.h"
#include "error.h"

int
libstm_init(const char *dbname, const char *pKey, int nKey, libstm_error_t *err)
{
    return libstm_db_init(dbname, pKey, nKey, NULL, err);
}
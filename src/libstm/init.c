#include "init.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "db.h"
#include "file.h"
#include "config.h"
#include "utils.h"
#include "sysuser.h"
#include "sec.h"

int
libstm_init(libstm_error_t *err)
{
    char *passwd = NULL;

    passwd = libstm_ask_password("setup database password: ", true, err);
    if (passwd == NULL)
        return STM_GENERIC_ERROR;

    return libstm_db_init(STM_DATABASE_NAME, passwd, strlen(passwd), NULL, err);
}
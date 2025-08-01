#include "init.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "db.h"
#include "file.h"
#include "config.h"
#include "utils.h"
#include "sec.h"

int
libstm_init(libstm_error_t *err)
{
    int rc = 0;
    char *passwd = NULL;
    char *prompt = "setup database password: ";

    passwd = libstm_ask_password(prompt, true, err);
    if (passwd == NULL)
        return STM_GENERIC_ERROR;

    rc = libstm_db_init(STM_DATABASE_NAME, passwd, strlen(passwd), NULL, err);
    if (rc < 0) {
        unlink(STM_DATABASE_NAME);
        return rc;
    }
    
    rc = libstm_db_create(STM_DATABASE_META, CREATE_DRYDB_META, err);
    if (rc < 0)
        unlink(STM_DATABASE_META);

    return rc;
}

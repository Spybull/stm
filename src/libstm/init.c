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
    int rc = 0;
    char *passwd = NULL;
    user_info_t *user = NULL;

    passwd = libstm_ask_password("setup database password: ", true, err);
    if (passwd == NULL)
        return STM_GENERIC_ERROR;

    rc = libstm_create_dir(STM_WORKDIR_PATH, 0750, err);
    if (stm_unlikely(rc < 0)) return rc;
    if (errno == EEXIST)
        fprintf(stderr, "INFO: directory `%s` already exists", STM_WORKDIR_PATH);

    user = libstm_setup_userinfo();
    if (stm_unlikely(user == NULL))
        return stm_make_error(err, 0, "failed to get user information");

    rc = chown(STM_WORKDIR_PATH, user->uid, user->gid);
    if (stm_unlikely(rc != 0))
        return stm_make_error(err, STM_GENERIC_ERROR, "failed to chown directory `%s`", STM_WORKDIR_PATH);
    
    return libstm_db_init(STM_DATABASE_NAME, passwd, strlen(passwd), NULL, err);
}
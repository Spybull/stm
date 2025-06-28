#include "init.h"
#include "stm.h"
#include <argp.h>

#include "libstm/init.h"
#include "libstm/file.h"
#include "libstm/utils.h"
#include "libstm/sec.h"

static char doc[] = "STM init";
static char args_doc[] = "init";
static struct argp argp = { NULL, NULL, args_doc, doc, NULL, NULL, NULL };

int
stm_command_init(stm_glob_args *glob_args stm_unused,
                int argc, char **argv, libstm_error_t *err)
{
    int rc = 0;
    argp_parse(&argp, argc, argv, 0, NULL, NULL);

    rc = stm_path_exists(DEFAULT_STM_DB_NAME, err);
    if (stm_unlikely(rc < 0))
        return rc;
    else if (rc == 1)
        libstm_fail_with_error(0, "database `%s` already exists", DEFAULT_STM_DB_NAME);

    /* ask user for password */
    char *passwd = stm_ask_password("setup database password: ", true, err);
    if (passwd == NULL)
        return -1;

    return libstm_init(DEFAULT_STM_DB_NAME, passwd, strlen(passwd), err);
}

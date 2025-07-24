#include "init.h"
#include "stm.h"
#include <argp.h>

#include "libstm/init.h"
#include "libstm/file.h"
#include "libstm/utils.h"
#include "libstm/sec.h"
#include "libstm/config.h"

static bool chpw = false;
enum { INIT_CHANGE_PASSWD = 'c' };
static struct argp_option options[] = {
    { "change", INIT_CHANGE_PASSWD, 0, 0, "Change database password", 0},
    { 0, }
};

static error_t
parse_opt(int key, char *arg stm_unused, struct argp_state *state stm_unused) {

    switch (key)
    {
        case INIT_CHANGE_PASSWD:
            chpw = true;
        break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}


static char doc[] = "STM init";
static char args_doc[] = "init";
static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

int
stm_command_init(stm_glob_args *glob_args,
                int argc, char **argv, libstm_error_t *err)
{
    int rc = 0;
    argp_parse(&argp, argc, argv, 0, 0, 0);

    rc = libstm_path_exists(STM_DATABASE_NAME, err);
    if (stm_unlikely(rc < 0))
        return rc;
    else if (rc == 1 && !chpw)
        return stm_make_error(err, 0, "nothing todo, already initialized");

    if (chpw)
    {
        // decrypt current database first:
        glob_args->pdb = libstm_db_auth(NULL, NULL, glob_args->stmd_creds_pid_path, glob_args->stmd_creds_sock_path, err);

        const char *passwd = libstm_ask_password("Enter new database password: ", true, err);
        if (passwd == NULL)
            return STM_GENERIC_ERROR;

        return libstm_db_rekey(glob_args->pdb, passwd, strlen(passwd), err);
    }

    return libstm_init(err);
}

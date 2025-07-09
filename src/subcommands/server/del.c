#include "del.h"
#include <argp.h>

#include "libstm/utils.h"
#include "libstm/sec.h"
#include "libstm/db.h"
#include "libstm/config.h"

static struct argp_option options[] = { { 0 } };
static error_t
parse_opt(int key, char *arg stm_unused, struct argp_state *state stm_unused) {

    switch (key)
    {
        case ARGP_KEY_NO_ARGS:
            libstm_fail_with_error(0, "please specify server name");

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char doc[] = "STM server delete";
static struct argp argp = { options, parse_opt, NULL, doc, NULL, NULL, NULL };

int
stm_server_subcmd_del(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
{
    int rc = 0, farg = 0;
    argp_parse(&argp, argc, argv, 0, &farg, NULL);

    glob_args->pdb = libstm_db_auth(NULL, NULL, glob_args->stmd_creds_pid_path, glob_args->stmd_creds_sock_path, err);
    if (!glob_args->pdb)
        return STM_GENERIC_ERROR;

    glob_args->mpdb = libstm_db_open(STM_DATABASE_META, NULL, err);
    if (stm_unlikely(glob_args->mpdb == NULL))
        return STM_GENERIC_ERROR;

    rc = libstm_db_server_del(glob_args->pdb, argv[farg], err);
    if (rc < 0)
        return STM_GENERIC_ERROR;
    
    libstm_db_server_del(glob_args->mpdb, argv[farg], err);
    return 0;
}
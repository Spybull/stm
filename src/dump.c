#include "dump.h"
#include <argp.h>

#include "libstm/db.h"
#include "libstm/utils.h"
#include "libstm/sec.h"
#include "libstm/formatter.h"

static struct argp_option options[] = { { 0, } };

static error_t
parse_opt(int key, char *arg stm_unused, struct argp_state *state stm_unused) {

    switch (key)
    {
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char doc[] = "STM dump";
static char args_doc[] = "dump";
static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

int
stm_command_dump(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
{
    int rc = 0;
    argp_parse(&argp, argc, argv, 0, 0, 0);

    glob_args->pdb = libstm_db_auth(NULL, NULL, glob_args->stmd_creds_pid_path, glob_args->stmd_creds_sock_path, err);
    rc = libstm_fmt_dump_as_csv(glob_args->pdb, err);

    return rc;
}
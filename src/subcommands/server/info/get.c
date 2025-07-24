#include "get.h"
#include <argp.h>

#include "libstm/db.h"
#include "libstm/sec.h"
#include "libstm/ssh.h"
#include "libstm/utils.h"
#include "libstm/compress.h"

static struct argp_option options[] = { { 0 } };
static error_t
parse_opt(int key, char *arg stm_unused, struct argp_state *state stm_unused)
{
    switch (key)
    {
        case ARGP_KEY_NO_ARGS:
            libstm_fail_with_error(0, "please specify server name");

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}
static char doc[] = "STM server info get";
static struct argp argp = { options, parse_opt, "NAME", doc, NULL, NULL, NULL };

stm_unused static char *inventory_matrix[] = {
    "cat /proc/sys/kernel/hostname",
    "cat /proc/cpuinfo",
    "cat /proc/meminfo",
    "cat /proc/net/dev",
    "cat /proc/net/route",
};

int
stm_server_info_get(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
{
    int farg = 0;
    libstm_server *srv = NULL;

    argp_parse(&argp, argc, argv, ARGP_IN_ORDER, &farg, 0);

    glob_args->pdb = libstm_db_auth(NULL, NULL,
    glob_args->stmd_creds_pid_path,
    glob_args->stmd_creds_sock_path, err);

    if (!glob_args->pdb)
        return STM_GENERIC_ERROR;

    srv = libstm_db_server_get(glob_args->pdb, argv[farg], err);
    if (srv == NULL)
        return STM_GENERIC_ERROR;

    ssh_session session = libstm_ssh_connect_once(srv->ip, srv->login, srv->creds, err);
    if (session == NULL)
        return -1;

    char *data = libstm_ssh_exec_cmd(&session, "uname -a");
    size_t dlen = strlen(data);
    return libstm_setup_server_info(glob_args->pdb, srv->name, "uname -a", data, dlen, err);
}
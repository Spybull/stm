#include "find.h"
#include <argp.h>
#include "libstm/utils.h"
#include "libstm/formatter.h"
#include "libstm/sec.h"

static const char *by_ip = NULL;
static const char *by_name = NULL;

enum
{
    BY_NAME = 'n',
    BY_IP = 'i'
};

static struct argp_option options[] = {
    { "name", BY_NAME, "STRING", 0, "find server by name", 0},
    { "ip",   BY_IP,   "STRING", 0, "find server by ip or hostname", 0},
    { 0, }
};

static error_t
parse_opt(int key, char *arg, struct argp_state *state stm_unused) {
    switch (key)
    {
        case BY_NAME: by_name = arg; break;
        case BY_IP:   by_ip = arg; break;
        default: break;
    }

    return 0;
}

static char doc[] = "STM server find";
static struct argp argp = { options, parse_opt, "NAME", doc, NULL, NULL, NULL };
int
stm_server_subcmd_find(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
{
    int rc = 0;
    libstm_server *srv = NULL;
    argp_parse(&argp, argc, argv, 0, 0, 0);

    glob_args->pdb = libstm_db_auth(NULL, NULL, glob_args->stmd_creds_pid_path, glob_args->stmd_creds_sock_path, err);
    if (!glob_args->pdb)
        return STM_GENERIC_ERROR;

    srv = libstm_db_server_get(glob_args->pdb, by_name, err);
    if (srv == NULL)
        return STM_GENERIC_ERROR;

    rc = stmlib_fmt_print_srv_as_json(srv, err);
    if (rc < 0)
        return STM_GENERIC_ERROR;

    libstm_server_free(srv);
    return 0;
}
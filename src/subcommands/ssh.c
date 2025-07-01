#include "ssh.h"
#include <argp.h>
#include "libstm/utils.h"

typedef struct {
    char *server;
    char *host;
    char **oargs; /* other args */
    int n_oargs;
}stm_ssh_arguments;

static error_t
parse_opt (int key, char *arg stm_unused, struct argp_state *state stm_unused)
{
    switch(key)
    {
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char doc[] = "STM ssh connection spawner";
static char args_doc[] = "[[user@]hostname] [ssh client options]";
static struct argp argp = { NULL, parse_opt, args_doc, doc, NULL, NULL, NULL };

int
stm_server_subcmd_ssh(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err stm_unused)
{
    stm_ssh_arguments ssh_arguments;
    argp_parse(&argp, argc, argv, 0, NULL, &ssh_arguments);

    return 0;
}
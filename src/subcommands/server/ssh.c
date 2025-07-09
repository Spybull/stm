#include "ssh.h"
#include <argp.h>

#include "libstm/utils.h"
#include "libstm/sec.h"
#include "libstm/ssh.h"





static error_t
parse_opt(int key, char *arg, struct argp_state *state) {
    
    libstm_server *args = state->input;

    switch (key)
    {
        case ARGP_KEY_ARG:
            if (state->arg_num == 0)
                args->name = arg;
            else
                return ARGP_ERR_UNKNOWN;
        break;
        case ARGP_KEY_NO_ARGS:
            libstm_fail_with_error(0, "please specify server name");
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}
static char doc[] = "STM ssh connection spawner";
static char args_doc[] = "[[user@]hostname] [ssh client options]";
static struct argp argp = { 0, parse_opt, args_doc, doc, NULL, NULL, NULL };

int
stm_server_subcmd_ssh(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
{
    libstm_server ssh_arguments = {0};
    argp_parse(&argp, argc, argv, 0, 0, &ssh_arguments);

    glob_args->pdb = libstm_db_auth(NULL, NULL, glob_args->stmd_creds_pid_path, glob_args->stmd_creds_sock_path, err);
    if (!glob_args->pdb)
        return STM_GENERIC_ERROR;

    libstm_server *srv = libstm_db_server_get(glob_args->pdb, ssh_arguments.name, err);
    if (srv == NULL)
        return STM_GENERIC_ERROR;

    return libstm_ssh_connect(srv, err);
}
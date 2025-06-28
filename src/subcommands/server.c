#include "server.h"
#include "libstm/utils.h"
#include <argp.h>



static error_t
parse_opt(int key, char *arg stm_unused, struct argp_state *state stm_unused) {

    switch (key)
    {
        case ARGP_KEY_NO_ARGS:
            libstm_fail_with_error(0, "please specify a subcommand");
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}
static char doc[] = "server";
static char args_doc[] = "server";
static struct argp run_argp = { NULL, NULL, args_doc, doc, NULL, NULL, NULL };


int
stm_subcommand_server(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
{
    return 0;
}
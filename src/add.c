#include "add.h"
#include <argp.h>

#include "libstm/utils.h"

static stm_glob_args arguments;

enum { CMD_SERVER = 1001 };
struct commands_s sub_cmds[] = {
    { CMD_SERVER, "server", implementation_stub },
    { 0, }
};
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

static char doc[] = "\nSUBCOMMANDS:\n"
                    "\tserver - add server\n";
static char args_doc[] = "add SUBCOMMAND";
static struct argp argp = { NULL, parse_opt, args_doc, doc, NULL, NULL, NULL };

int
stm_command_add(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err)
{
    int rc = 0, farg = 0;
    struct commands_s *curr_cmd;

    arguments.argc = argc;
    arguments.argv = argv;

    argp_parse(&argp, argc, argv, ARGP_IN_ORDER, &farg, &arguments);
    curr_cmd = stm_get_command(argv[farg], sub_cmds);
    if (!curr_cmd)
        libstm_fail_with_error(0, "unknown subcommand `%s`", argv[farg]);

    rc = curr_cmd->handler(&arguments, argc - farg, argv + farg, err);
    if (rc < 0)
        libstm_fail_with_error(0, "add error");
    return 0;
}
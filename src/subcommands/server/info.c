#include "info.h"
#include <argp.h>

#include "libstm/utils.h"
#include "info/get.h"

enum { SERVER_INFO_GET = 1001 };
static struct argp_option options[] = {{ 0 }};
static struct commands_s sub_cmds[] = {
    { SERVER_INFO_GET,  "get",  stm_server_info_get },
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
                    "\tget  - get server info\n";
static char args_doc[] = "SUBCOMMAND";
static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

int
stm_server_subcmd_info(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
{
    int rc = 0, farg = 0;
    struct commands_s *curr_cmd;

    argp_parse(&argp, argc, argv, ARGP_IN_ORDER, &farg, 0);

    curr_cmd = stm_get_command(argv[farg], sub_cmds);
    if (!curr_cmd)
        libstm_fail_with_error(0, "unknown subcommand `%s`", argv[farg]);

    rc = curr_cmd->handler(glob_args, argc - farg, argv + farg, err);
    if (rc < 0 && (*err))
        libstm_fail_with_error((*err)->status, "%s", (*err)->msg);

    return 0;
}
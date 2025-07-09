#include "creds.h"
#include <argp.h>

#include "libstm/utils.h"
#include "subcommands/creds/store.h"
#include "subcommands/creds/status.h"
#include "subcommands/creds/kill.h"

enum { CREDS_STORE = 1001, CREDS_STATUS, CREDS_KILL };
static struct commands_s sub_cmds[] = {
    { CREDS_STORE,   "store",  stm_creds_subcmd_store  },
    { CREDS_STATUS,  "status", stm_creds_subcmd_status },
    { CREDS_KILL,    "kill",   stm_creds_subcmd_kill   },
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

static char doc[] = "\nSUB COMMANDS:\n"
                    "\tstore  - store database creds\n"
                    "\tstatus - get daemon status\n"
                    "\tkill   - kill creds daemon\n";
static char args_doc[] = "name";
static struct argp argp = { NULL, parse_opt, args_doc, doc, NULL, NULL, NULL };

int
stm_command_creds(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
{
    int farg = 0;
    struct commands_s *curr_cmd;

    argp_parse(&argp, argc, argv, ARGP_IN_ORDER, &farg, 0);

    curr_cmd = stm_get_command(argv[farg], sub_cmds);
    if (!curr_cmd)
        libstm_fail_with_error(0, "unknown subcommand `%s`", argv[farg]);
    
    return curr_cmd->handler(glob_args, argc - farg, argv + farg, err);
}
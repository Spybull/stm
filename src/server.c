#include "server.h"
#include <argp.h>


#include "libstm/db.h"
#include "libstm/utils.h"
#include "libstm/sec.h"

#include "subcommands/server/add.h"
#include "subcommands/server/ssh.h"
#include "subcommands/server/del.h"
#include "subcommands/server/list.h"
#include "subcommands/server/find.h"
#include "subcommands/server/info.h"

enum { SERVER_ADD = 1001, SERVER_LIST, SERVER_DEL, SERVER_FIND, SERVER_INFO };
static struct commands_s sub_cmds[] = {
    { SERVER_ADD,  "add",  stm_server_subcmd_add  },
    { SERVER_ADD,  "ssh",  stm_server_subcmd_ssh  },
    { SERVER_DEL,  "del",  stm_server_subcmd_del  },
    { SERVER_INFO, "info", stm_server_subcmd_info },
    { SERVER_LIST, "list", stm_server_subcmd_list },
    { SERVER_FIND, "find", stm_server_subcmd_find },
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
                    "\tadd  - add server\n"
                    "\tssh  - connect to server\n"
                    "\tdel  - delete server\n"
                    "\tlist - list servers\n"
                    "\tfind - find servers\n"
                    "\tinfo - get server info\n";
static char args_doc[] = "name";
static struct argp argp = { NULL, parse_opt, args_doc, doc, NULL, NULL, NULL };

int
stm_command_server(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
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
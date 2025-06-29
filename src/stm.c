#include "stm.h"
#include <stdio.h>
#include <stdlib.h>

#include "libstm/error.h"
#include "libstm/utils.h"
#include "libstm/sysuser.h"
#include "libstm/file.h"
#include "libstm/config.h"

/* STM HELPER FUNCTIONS */
#include "cmd.h"

/* STM COMMANDS IMPL */
#include "init.h"
#include "add.h"

const char *argp_program_bug_address = "https://github.com/Spybull/stm/issues";
static stm_glob_args arguments;
enum { CMD_INIT = 1001, CMD_ADD };
struct commands_s cmds[] = {
    { CMD_INIT, "init", stm_command_init},
    { CMD_ADD,  "add" , stm_command_add },
    { 0, }
};

static error_t
parse_opt(int key, char *arg stm_unused, struct argp_state *state stm_unused) {

    switch (key)
    {
        case ARGP_KEY_NO_ARGS:
            libstm_fail_with_error(0, "please specify a command");
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char args_doc[] = "COMMAND [OPTION...]";
static char doc[] = "\nCOMMANDS:\n"
                    "\tinit - initialize database\n"
                    "\tadd  - add something\n";

static struct argp argp = { NULL, parse_opt, args_doc, doc, NULL, NULL, NULL };

// static int
// stm_init_context(libstm_error_t *err)
// {
//     int rc;
//     cleanup_free char *pwd = NULL;
//     pwd = realpath(".", NULL);

//     user_info_t *current_user = libstm_setup_userinfo();

//     // rc = libstm_path_exists(STM_WORKDIR_PATH, err);
//     // if (rc < 0)
//     //     return rc;
//     // if (rc == 0)
        
//     // else 

// }

int main(int argc, char **argv)
{
    int rc = 0, farg = 0;
    libstm_error_t err = NULL;
    struct commands_s *curr_cmd;

    arguments.argc = argc;
    arguments.argv = argv;

    argp_parse(&argp, argc, argv, ARGP_IN_ORDER, &farg, &arguments);
    curr_cmd = stm_get_command(argv[farg], cmds);
    if (!curr_cmd)
        libstm_fail_with_error(0, "unknown command `%s`", argv[farg]);

    if (chdir(STM_WORKDIR_PATH) < 0)
        libstm_fail_with_error(errno, "failed to change directory `%s` ",
                              STM_WORKDIR_PATH);

    rc = curr_cmd->handler(&arguments, argc - farg, argv + farg, &err);
    if (rc < 0 && err)
        libstm_fail_with_error(err->status, "%s", err->msg);
    return 0;
}
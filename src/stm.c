#include "stm.h"
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>

#include "libstm/error.h"
#include "libstm/utils.h"
#include "libstm/sysuser.h"
#include "libstm/file.h"
#include "libstm/config.h"
#include "libstm/db.h"

/* STM HELPER FUNCTIONS */
#include "cmd.h"

/* STM COMMANDS IMPL */
#include "init.h"
#include "creds.h"
#include "server.h"

const char *argp_program_bug_address = "https://github.com/Spybull/stm/issues";
static stm_glob_args arguments;
enum { 
    CMD_INIT = 1001,
    CMD_CREDS,
    CMD_SERVER
};

struct commands_s cmds[] = {
    { CMD_INIT,   "init",   stm_command_init   },
    { CMD_CREDS,  "creds",  stm_command_creds  },
    { CMD_SERVER, "server", stm_command_server },
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
                    "\tinit   - initialize database\n"
                    "\tcreds  - credential manager\n"
                    "\tserver - manage servers\n";

static struct argp argp = { NULL, parse_opt, args_doc, doc, NULL, NULL, NULL };

int main(int argc, char **argv)
{
    int rc = 0, farg = 0;
    libstm_error_t err = NULL;
    struct commands_s *curr_cmd;
    char user_home[PATH_MAX],
         stm_workdir[PATH_MAX];

    arguments.argc = argc;
    arguments.argv = argv;

    argp_parse(&argp, argc, argv, ARGP_IN_ORDER, &farg, &arguments);
    curr_cmd = stm_get_command(argv[farg], cmds);
    if (!curr_cmd)
        libstm_fail_with_error(0, "unknown command `%s`", argv[farg]);

    rc = libstm_get_workdir(user_home, &err);
    if (stm_unlikely(rc < 0))
        goto exit_fail;

    rc = safe_path(stm_workdir, PATH_MAX, "%s/%s", user_home, STM_USER_DIR_NAME);
    if (stm_unlikely(rc < 0))
        libstm_fail_with_error(0, "path `%s/%s` > PATH_MAX(%d)", user_home, STM_USER_DIR_NAME, PATH_MAX);

    rc = libstm_create_dir(stm_workdir, 0700, &err);
    if (stm_unlikely(rc < 0))
        goto exit_fail;

    if (chdir(stm_workdir) < 0)
        libstm_fail_with_error(errno, "failed to change directory `%s` ", STM_SYSDIR_PATH);

    rc = curr_cmd->handler(&arguments, argc - farg, argv + farg, &err);
    if (!arguments.pdb)
        sqlite3_close_v2(arguments.pdb);
    if (!arguments.mpdb)
        sqlite3_close_v2(arguments.mpdb);

exit_fail:
    if (rc < 0 && err)
        libstm_fail_with_error(err->status, "%s", err->msg);
    return 0;
}
#include "stm.h"
#include <stdio.h>
#include <stdlib.h>

#include "libstm/error.h"
#include "libstm/utils.h"
#include "init.h"

static stm_glob_args arguments;
struct commands_s {
    int value;
    const char *name;
    int (*handler) (stm_glob_args *, int, char **, libstm_error_t *);
};

enum
{
    CMD_INIT = 1001,
    CMD_ADD
};

struct commands_s cmds[] = {
    { CMD_INIT, "init", stm_command_init},
    { 0, }
};

static struct commands_s *get_command(const char *arg) {
    struct commands_s *it;
    for (it = cmds; it->value; it++)
        if (strcmp(it->name, arg) == 0)
            return it;

    return NULL;
}

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
                    "\tinit - initialize database\n";
static struct argp argp = { NULL, parse_opt, args_doc, doc, NULL, NULL, NULL };

int main(int argc, char **argv)
{
    int rc = 0, farg = 0;
    libstm_error_t err = NULL;
    struct commands_s *curr_cmd;

    arguments.argc = argc;
    arguments.argv = argv;

    argp_parse(&argp, argc, argv, ARGP_IN_ORDER, &farg, &arguments);
    curr_cmd = get_command(argv[farg]);
    if (!curr_cmd)
        libstm_fail_with_error(0, "unknown command `%s`", argv[farg]);

    rc = curr_cmd->handler(&arguments, argc - farg, argv + farg, &err);
    if (rc < 0 && err)
        libstm_fail_with_error(err->status, "%s", err->msg);
    return 0;
}
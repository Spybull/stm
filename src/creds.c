#include "creds.h"
#include <argp.h>
#include "libstm/utils.h"

static char args_doc[] = "SUB COMMAND";
static char doc[] = "\nSUB COMMANDS:\n"
                    "\tstore - store database creds\n";
static struct argp argp = { NULL, NULL, args_doc, doc, NULL, NULL, NULL };

int
stm_command_creds(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err stm_unused)
{
    argp_parse(&argp, argc, argv, 0, 0, 0);
    return 0;
}
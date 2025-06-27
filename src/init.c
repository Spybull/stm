#include "init.h"
#include <argp.h>

#include "libstm/init.h"
#include "libstm/utils.h"

static char doc[] = "STM init";
static char args_doc[] = "init";
static struct argp run_argp = { NULL, NULL, args_doc, doc, NULL, NULL, NULL };

int
stm_command_init(stm_glob_args *glob_args stm_unused,
                int argc, char **argv, libstm_error_t *err)
{
    argp_parse(&run_argp, argc, argv, 0, NULL, NULL);
    return libstm_init(err);
}

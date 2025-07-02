#include "store.h"
#include <argp.h>
#include "libstm/utils.h"


static char doc[] = "STM creds store";
static struct argp argp = { 0, 0, "NAME", doc, NULL, NULL, NULL };
int
stm_creds_subcmd_store(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err stm_unused)
{
    argp_parse(&argp, argc, argv, 0, 0, 0);
    return 0;
}
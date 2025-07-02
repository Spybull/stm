#include "init.h"
#include "stm.h"
#include <argp.h>

#include "libstm/init.h"
#include "libstm/file.h"
#include "libstm/utils.h"
#include "libstm/sec.h"
#include "libstm/config.h"

static char doc[] = "STM init";
static char args_doc[] = "init";
static struct argp argp = { NULL, NULL, args_doc, doc, NULL, NULL, NULL };

int
stm_command_init(stm_glob_args *glob_args stm_unused,
                int argc, char **argv, libstm_error_t *err)
{
    int rc = 0;
    argp_parse(&argp, argc, argv, 0, 0, 0);

    rc = libstm_path_exists(STM_DATABASE_NAME, err);
    if (stm_unlikely(rc < 0))
        return rc;
    else if (rc == 1)
        return stm_make_error(err, 0, "nothing todo, already initialized");

    return libstm_init(err);
}

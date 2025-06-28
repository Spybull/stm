#include "cmd.h"

#include "libstm/utils.h"
#include <string.h>

struct commands_s *
stm_get_command(const char *arg, struct commands_s cmds[]) {
    struct commands_s *it;
    for (it = cmds; it->value; it++)
        if (strcmp(it->name, arg) == 0)
            return it;

    return NULL;
}

int
implementation_stub(stm_glob_args *global_args stm_unused,
                    int argc stm_unused, char **argv,
                    libstm_error_t *err stm_unused)
{
    libstm_fail_with_error(0, "command `%s` not implemented", argv[0]);
}
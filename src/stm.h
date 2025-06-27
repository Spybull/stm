#ifndef _STM_H
#define _STM_H

#include <argp.h>
#include <stdbool.h>
#include "libstm/error.h"

struct stm_global_arguments {
    int argc;
    char **argv;
    int (*auth_handler)(const char *prompt, char *pwout, libstm_error_t *err);
};
typedef struct stm_global_arguments stm_glob_args;

#endif
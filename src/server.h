#ifndef STM_SUBCMD_SERVER_H
#define STM_SUBCMD_SERVER_H

#include "cmd.h"
#include "libstm/error.h"

int stm_command_server(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err);
#endif
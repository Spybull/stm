#ifndef SSH_H
#define SSH_H
#include "cmd.h"

int stm_server_subcmd_ssh(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err);

#endif
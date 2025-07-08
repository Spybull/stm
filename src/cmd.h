#ifndef LIBSTM_CMD_H
#define LIBSTM_CMD_H

#include "libstm/error.h"
#include "libstm/db.h"

struct stm_global_arguments {
    int argc;
    char **argv;
    sqlite3 *pdb, *mpdb;
};
typedef struct stm_global_arguments stm_glob_args;

struct commands_s {
    int value;
    const char *name;
    int (*handler) (stm_glob_args *, int, char **, libstm_error_t *);
};

struct commands_s *stm_get_command(const char *arg, struct commands_s cmds[]);
int implementation_stub(stm_glob_args *global_args, int argc, char **argv, libstm_error_t *err);


#endif
#ifndef STM_FILE_H
#define STM_FILE_H

#include "error.h"

int stm_create_file(const char *path, libstm_error_t *err);
int stm_path_exists(const char *path, libstm_error_t *err);
#endif
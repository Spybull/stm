#ifndef STM_FILE_H
#define STM_FILE_H

#include "error.h"
#include <sys/stat.h>


int libstm_create_file(const char *path, libstm_error_t *err);
int libstm_create_dir(const char *path, mode_t mode, libstm_error_t *err);
int libstm_is_dir(const char *path, struct stat *st, libstm_error_t *err);
int libstm_path_exists(const char *path, libstm_error_t *err);
#endif
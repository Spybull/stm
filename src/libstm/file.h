#ifndef STM_FILE_H
#define STM_FILE_H

#include "error.h"
#include <sys/stat.h>


STM_HIDDEN int libstm_is_dir(const char *path, struct stat *st, libstm_error_t *err);
STM_HIDDEN int libstm_is_file_locked(int fd, libstm_error_t *err);

STM_API int libstm_create_file(const char *path, mode_t mode, libstm_error_t *err);
STM_API int libstm_create_dir(const char *path, mode_t mode, libstm_error_t *err);
STM_API int libstm_path_exists(const char *path, libstm_error_t *err);
STM_API int libstm_lock_file(int fd);
STM_API int safe_path(char *buf, size_t size, const char *fmt, ...);
STM_API int libstm_get_workdir(char *out, libstm_error_t *err);
STM_API pid_t read_pid_file(const char *lock_file, libstm_error_t *err);
STM_API FILE *xfdopen(int fd, const char *mode);

#endif

#include "file.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdarg.h>

int
libstm_create_file(const char *path, libstm_error_t *err) {
    cleanup_close int fd = -1;
    fd = open(path, O_CREAT | O_EXCL, 0666);
    if (fd < 0 && errno != EEXIST)
        return stm_make_error(err, errno, "failed to create file `%s`", path);
    return 0;
}

/* return 1 if path exists; return 0 if path doesn't exist else error*/
int
libstm_path_exists(const char *path, libstm_error_t *err) {
    int ret = access(path, F_OK);
    if (ret < 0) {
        if (errno == ENOENT)
            return 0;
        return stm_make_error(err, errno, "access `%s`", path);
    }
    return 1;
}

/* If path is not a direcotry - zero is returned, 1 if it's a directory.
   On error, -1 is returned, and errno is set to indicate the error. */
static int
is_dir(const char *path, struct stat *st)
{
    int rc;
    struct stat _st;

    if (st == NULL)
        st = &_st;

    rc = stat(path, st);
    if (stm_unlikely(rc < 0))
        return rc;
    
    return S_ISDIR(st->st_mode);
}

int
libstm_is_dir(const char *path, struct stat *st, libstm_error_t *err)
{
    int rc = is_dir(path, st);
    if (rc <= 0)
        return stm_make_error(err, errno, "`%s`", path);
    return rc;
}


int
libstm_create_dir(const char *path, mode_t mode, libstm_error_t *err) {

    int rc = 0;

    rc = mkdir(path, mode);
    if (stm_unlikely(rc < 0)) {
        if (errno == EEXIST) {
            rc = libstm_is_dir(path, NULL, err);
            if (rc < 0)
                return rc;

            if (rc == 0) /* errno == ENOTDIR */
                return stm_make_error(err, errno, "not a directory"); /* not a directory */

            return 0; /* directory already exists */
        }
        return stm_make_error(err, errno, "failed to create directory `%s`", path);
    }

    return 0;
}

int
safe_path(char *buf, size_t size, const char *fmt, ...)
{
    va_list args_list;
    va_start(args_list, fmt);

    int n = vsnprintf(NULL, 0, fmt, args_list);
    va_end(args_list);

    if (n < 0 || (size_t)n >= size)
        return -1;

    va_start(args_list, fmt);
    vsnprintf(buf, size, fmt, args_list);
    va_end(args_list);
    return 0;
}

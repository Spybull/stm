#include "file.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>

int
libstm_create_file(const char *path, mode_t mode, libstm_error_t *err)
{
    cleanup_close int fd = -1;
    fd = open(path, O_CREAT | O_EXCL, mode);
    if (fd < 0 && errno != EEXIST)
        return stm_make_error(err, errno, "failed to create file `%s`", path);
    return 0;
}

/* return 1 if path exists; return 0 if path doesn't exist else error*/
int
libstm_path_exists(const char *path, libstm_error_t *err)
{
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
        return stm_make_error(err, errno, "failed to create directory `%s` ", path);
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

/* 1 - locked, 0 - not locked, < 0 - error */
int
libstm_is_file_locked(int fd, libstm_error_t *err)
{
    struct flock lock;
    
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;

    if (fcntl(fd, F_GETLK, &lock) < 0)
        return stm_make_error(err, errno, "failed to check lock on file in %s", __func__);

    return !(lock.l_type == F_UNLCK);
}

int
libstm_lock_file(int fd) {

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    return (fcntl(fd, F_SETLK, &lock));
}

FILE *
xfdopen(int fd, const char *mode) {

    FILE *stream = fdopen(fd, mode); 
    if (stm_unlikely(stream == NULL))
        stm_oom();
    return stream;
}

pid_t
read_pid_file(const char *lock_file, libstm_error_t *err) {
    
    char buf[16] = {0};
    cleanup_close int fd = 0;

    fd = open(lock_file, O_RDONLY);
    if (fd < 0) {
        if (errno == ENOENT) {
            return stm_make_error(err, 0, "stm creds daemon is not running");
        }

        return stm_make_error(err, errno, "failed to open %s", lock_file);
    }

    ssize_t nr = 0;
    if ( ( nr = read(fd, buf, sizeof(buf))) < 0 )
        return stm_make_error(err, errno, "failed to read %s", lock_file);

    if (nr == 0)
        return stm_make_error(err, 0, "no such process pid in %s", lock_file);

    return (pid_t)atoi(buf);
}


int
libstm_get_workdir(char *out, libstm_error_t *err)
{
    cleanup_free char *home_path = NULL;

    const char *env = getenv("HOME");
    if (stm_likely(env != NULL)) {
        home_path = xstrdup(env);
    } else {
        home_path = getcwd(NULL, 0);
        if (stm_unlikely(home_path == NULL))
            return stm_make_error(err, errno, "failed to getcwd");
    }

    snprintf(out, PATH_MAX, "%s", home_path);

    return 0;
}
#include "file.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <unistd.h>

int
stm_create_file(const char *path, libstm_error_t *err) {
    cleanup_close int fd = -1;
    fd = open(path, O_CREAT | O_EXCL, 0666);
    if (fd < 0 && errno != EEXIST)
        return stm_make_error(err, errno, "failed to create file `%s`", path);
    return 0;
}

/* return 1 if path exists; return 0 if path doesn't exist else error*/
int
stm_path_exists(const char *path, libstm_error_t *err) {
    int ret = access(path, F_OK);
    if (ret < 0) {
        if (errno == ENOENT)
            return 0;
        return stm_make_error(err, errno, "access `%s`", path);
    }
    return 1;
}
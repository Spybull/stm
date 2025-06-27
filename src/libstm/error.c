#include "error.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "utils.h"

int
libstm_make_error(libstm_error_t *err, int status, const char *msg, ...)
{
    va_list args_list;
	libstm_error_t ptr;

    va_start(args_list, msg);
    *err = xmalloc(sizeof(struct libstm_error_s));
    ptr = *err;
    ptr->status = status;
	if (vasprintf(&(ptr->msg), msg, args_list) < 0)
	    stm_oom();
    va_end(args_list);
    return -status - 1;
}

void
libstm_fail_with_error(int errno_, const char *msg, ...)
{
    int rc;
    cleanup_free char *out = NULL;

    va_list args_list;
    va_start(args_list, msg);

    rc = vasprintf(&out, msg, args_list);
    if (stm_unlikely(rc < 0))
        stm_oom();

    if (errno_ != 0)
        fprintf(stderr, "%s%s%s%s\n", "\x1b[1;31m", out, strerror(errno_), "\x1b[0m");
    else
        fprintf(stderr, "%s%s%s\n", "\x1b[1;31m", out, "\x1b[0m");
    va_end(args_list);
    exit(EXIT_FAILURE);
}
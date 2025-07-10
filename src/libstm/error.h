#ifndef STM_ERROR_H
#define STM_ERROR_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <syslog.h>

#define STM_GENERIC_ERROR -1
#define STM_COMPRESSION_ERROR -10

/* syslog */
#define UERR LOG_USER | LOG_ERR
#define UINF LOG_USER | LOG_INFO

struct libstm_error_s {
  int status;
  char *msg;
};
typedef struct libstm_error_s *libstm_error_t;

void libstm_fail_with_error(int errno_, const char *msg, ...) __attribute__ ((noreturn)) __attribute__ ((format (printf, 2, 3)));
int libstm_make_error(libstm_error_t *err, int status, const char *msg, ...) __attribute__ ((format (printf, 3, 4)));
#define stm_make_error libstm_make_error

#endif
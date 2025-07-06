#ifndef STM_SSG_H
#define STM_SSG_H
#include "db.h"

struct libstm_ssh_user_data_s {
    char *login;
    char *host;
};
typedef struct libstm_ssh_user_data_s ssh_user_data_t;

int libstm_ssh_connect(libstm_server *srv, libstm_error_t *err);
ssh_user_data_t *libstm_parse_user_host(const char *user_host, libstm_error_t *err);
#endif
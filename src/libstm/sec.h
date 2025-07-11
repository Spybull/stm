#ifndef STM_SEC_H
#define STM_SEC_H
#include "error.h"
#include <sqlite3.h>

STM_API char *libstm_ask_password(const char *prompt, int verify, libstm_error_t *err);
STM_API sqlite3 *libstm_db_auth(const char *prompt, char *pwout, const char *daemon_pid, const char *daemon_socket, libstm_error_t *err);

#endif
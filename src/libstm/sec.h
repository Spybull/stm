#ifndef STM_SEC_H
#define STM_SEC_H
#include "error.h"
#include <sqlite3.h>

char *libstm_ask_password(const char *prompt, int verify, libstm_error_t *err);
sqlite3 *libstm_db_auth(const char *prompt, char *pwout, libstm_error_t *err);
#endif
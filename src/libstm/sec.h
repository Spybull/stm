#ifndef STM_SEC_H
#define STM_SEC_H
#include "error.h"

char *libstm_ask_password(const char *prompt, int verify, libstm_error_t *err);
int libstm_auth(const char *prompt, char *pwout, libstm_error_t *err);
#endif
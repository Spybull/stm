#ifndef STM_SEC_H
#define STM_SEC_H
#include "error.h"

char *stm_ask_password(const char *prompt, int verify, libstm_error_t *err);

#endif
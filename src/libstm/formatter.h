#ifndef STM_FORMATTER_H
#define STM_FORMATTER_H
#include "error.h"
#include <sqlite3.h>

int stmlib_fmt_print_json(sqlite3 *pdb, const char *query, libstm_error_t *err);

#endif
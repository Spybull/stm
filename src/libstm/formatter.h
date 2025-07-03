#ifndef STM_FORMATTER_H
#define STM_FORMATTER_H
#include "error.h"
#include <stdbool.h>
#include <sqlite3.h>

int stmlib_fmt_print_json(sqlite3 *pdb, const char *query, libstm_error_t *err);
int stmlib_fmt_print_csv(sqlite3 *pdb, const char *query, bool with_header, libstm_error_t *err);
#endif
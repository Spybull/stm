#ifndef STM_FORMATTER_H
#define STM_FORMATTER_H
#include <stdbool.h>
#include <sqlite3.h>
#include "db.h"

STM_API int libstm_fmt_print_json(sqlite3 *pdb, const char *query, libstm_error_t *err);
STM_API int libstm_fmt_print_csv(sqlite3 *pdb, const char *query, bool with_header, libstm_error_t *err);
STM_API int libstm_fmt_print_srv_as_json(libstm_server *srv, libstm_error_t *err);
STM_API int libstm_fmt_dump_as_csv(sqlite3 *pdb, libstm_error_t *err);
#endif
#include "formatter.h"
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include "utils.h"

int
stmlib_fmt_print_json(sqlite3 *pdb, const char *query, libstm_error_t *err)
{
    int rc = 0;
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(pdb, query, -1, &stmt, NULL);
    if (stm_unlikely(rc != SQLITE_OK))
        return stm_make_error(err, errno, "failed to prepare sql query: %s", sqlite3_errmsg(pdb));

    int cols = sqlite3_column_count(stmt);
    json_t *jarr = json_array();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        json_t *row = json_object();
        for (int i = 0; i < cols; ++i) {
            const char *col = sqlite3_column_name(stmt, i);
            if (strcmp(col, "id") == 0 || strcmp(col, "created_at") == 0)
                continue;
            switch (sqlite3_column_type(stmt, i)) {
                case SQLITE_INTEGER:
                    json_object_set_new(row, col, json_integer(sqlite3_column_int64(stmt, i)));
                    break;
                case SQLITE_FLOAT:
                    json_object_set_new(row, col, json_real(sqlite3_column_double(stmt, i)));
                    break;
                case SQLITE_TEXT:
                    json_object_set_new(row, col, json_string((const char *)sqlite3_column_text(stmt, i)));
                    break;
                case SQLITE_NULL:
                    json_object_set_new(row, col, json_null());
                    break;
                default:
                    json_object_set_new(row, col, json_string("UNSUPPORTED_TYPE"));
            }
        }
        json_array_append_new(jarr, row);
    }

    char *out = json_dumps(jarr, JSON_INDENT(2));
    if (stm_unlikely(out == NULL))
        return stm_make_error(err, 0, "failed in `json_dumps`");
    printf("%s\n", out);


    free(out);
    json_decref(jarr);
    sqlite3_finalize(stmt);
    return 0;

}
#include "formatter.h"
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>

#include "utils.h"
#include "queries.h"

static json_t *
json_string_or_json_null(const char *val)
{
    return val == NULL  ? json_null() : json_string(val);
}


int
libstm_fmt_print_srv_as_json(libstm_server *srv, libstm_error_t *err)
{
    json_t *row = json_object();
    json_object_set_new(row, "name", json_string_or_json_null(srv->name));
    json_object_set_new(row, "ip", json_string_or_json_null(srv->addr));
    json_object_set_new(row, "port", json_integer(srv->port));
    json_object_set_new(row, "proto", json_string_or_json_null(srv->proto));
    json_object_set_new(row, "login", json_string_or_json_null(srv->login));
    json_object_set_new(row, "creds", json_string_or_json_null(srv->creds));
    json_object_set_new(row, "description", json_string_or_json_null(srv->description));

    char *out = json_dumps(row, JSON_INDENT(2));
    if (stm_unlikely(out == NULL))
        return stm_make_error(err, 0, "failed in `json_dumps`");
    printf("%s\n", out);


    free(out);
    json_decref(row);
    return 0;
}

int
libstm_fmt_print_json(sqlite3 *pdb, const char *query, libstm_error_t *err)
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

int
libstm_fmt_print_csv(sqlite3 *pdb, const char *query, bool with_header, libstm_error_t *err)
{
    int rc = 0;
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(pdb, query, -1, &stmt, NULL);
    if (stm_unlikely(rc != SQLITE_OK))
        return stm_make_error(err, errno, "failed to prepare sql query: %s", sqlite3_errmsg(pdb));

    int cols = sqlite3_column_count(stmt);

    int include_cols[cols];
    int out_cols = 0;
    for (int i = 0; i < cols; ++i) {
        const char *col = sqlite3_column_name(stmt, i);
        if (strcmp(col, "id") != 0 && strcmp(col, "created_at") != 0) {
            include_cols[out_cols++] = i;
        }
    }

    if (with_header) {
        for (int i = 0; i < out_cols; ++i) {
            const char *col = sqlite3_column_name(stmt, include_cols[i]);
            printf("%s%s", col, (i < out_cols - 1) ? "," : "\n");
        }
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        for (int i = 0; i < out_cols; ++i) {
            int col_index = include_cols[i];
            if (sqlite3_column_type(stmt, col_index) == SQLITE_NULL) {
                printf("\"\"%s", (i < out_cols - 1) ? "," : "\n");
            } else {
                const unsigned char *raw = sqlite3_column_text(stmt, col_index);
                printf("\"");
                for (const unsigned char *p = raw; *p; ++p) {
                    if (*p == '"') putchar('"');
                    putchar(*p);
                }
                printf("\"%s", (i < out_cols - 1) ? "," : "\n");
            }
        }
    }

    sqlite3_finalize(stmt);
    return 0;
}

int libstm_fmt_dump_as_csv(sqlite3 *pdb, libstm_error_t *err) {
    return libstm_fmt_print_csv(pdb, SELECT_ALL_FROM_SERVERS, false, err);
}
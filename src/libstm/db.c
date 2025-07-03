#include "db.h"
#include "file.h"
#include "utils.h"
#include "config.h"
#include "queries.h"
#include <stdbool.h>

static int
make_dummy_query(sqlite3 *pdb, libstm_error_t *err) {
    
    int rc = 0;
    cleanup_free char *pwd = NULL;

    pwd = getcwd(NULL, 0);
    rc = sqlite3_exec(pdb, "SELECT count(*) FROM sqlite_master;", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        if (stm_likely(rc == SQLITE_NOTADB))
            return stm_make_error(err, SQLITE_NOTADB, "failed to decrypt database file `%s/%s`",
                                   pwd, STM_DATABASE_NAME);

        return stm_make_error(err, 0, "failed to query database: `%s`", sqlite3_errmsg(pdb));
    }

    return 0;
}

static inline int
bind_text_by_str(sqlite3_stmt *stmt, const char *text, const char *zName, bool is_static)
{
    int idx = -1;
    idx = sqlite3_bind_parameter_index(stmt, zName);
    return sqlite3_bind_text(stmt, idx, text, -1, is_static ? SQLITE_STATIC : SQLITE_TRANSIENT);
}

static inline int
bind_int_by_str(sqlite3_stmt *stmt, int data, const char *zName)
{
    int idx = -1;
    idx = sqlite3_bind_parameter_index(stmt, zName);
    return sqlite3_bind_int(stmt, idx, data);
}

static int
doquery_with_callback(sqlite3 *pdb, const char *query, sqlite_cb cb, void *param)
{
    int rc = 0;
    rc = sqlite3_exec(pdb, query, cb, param, NULL);
    return (rc == SQLITE_OK) ? 0 : -1;
}

sqlite3 *
libstm_db_open(const char *filename, const char *pKey, libstm_error_t *err)
{
    int rc;
    sqlite3 *pdb = NULL;

    if (filename != NULL && stm_unlikely(access(filename, F_OK) < 0)) {
        stm_make_error(err, errno, "database `%s` does not exists ", filename);
        return NULL;
    }

    rc = sqlite3_open_v2(filename, &pdb, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK) {
        stm_make_error(err, errno, "failed to open database `%s`. ", filename);
        return NULL;
    }

    if (pKey) {
        rc = libstm_db_decrypt(pdb, pKey, strlen(pKey), err);
        if (rc != SQLITE_OK) {
            stm_make_error(err, 0, "%s", "failed to decrypt database");
            return NULL;
        }
        /* TODO: make dummy query after decryption */
    }
    
    return pdb;
}

int
libstm_db_init(const char *filename, const char *pKey, int nKey, const char *scheme, libstm_error_t *err)
{
    int rc;
    sqlite3 *pdb;

    rc = libstm_create_file(filename, 0666, err);
    if (stm_unlikely(rc < 0))
        return rc;

    rc = sqlite3_open_v2(filename, &pdb, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
        return stm_make_error(err, errno, "failed to open database `%s`", filename);

    rc = sqlite3_key_v2(pdb, filename, pKey, nKey);
    if (rc != SQLITE_OK)
        return stm_make_error(err, rc, "failed to setup encryption key");

    if (!scheme)
        scheme = CREATE_DRYDB;

    rc = sqlite3_exec(pdb, scheme, NULL, NULL, NULL);
    if (rc != SQLITE_OK)
        return stm_make_error(err, 0, "failed to create default schemes, %s", sqlite3_errmsg(pdb));

    sqlite3_close_v2(pdb);
    return 0;
}

int
libstm_db_decrypt(sqlite3 *pdb, const char *pKey, int nKey, libstm_error_t *err)
{
    int rc;
    sqlite3_key_v2(pdb, NULL, pKey, nKey);    
    rc = make_dummy_query(pdb, err);
    if (rc < 0)
        return rc;
    return 0;
}

static int
add_server(sqlite3 *pdb, const char *sql, libstm_server *srv, libstm_error_t *err)
{
    int rc = 0;
    sqlite3_stmt *stmt = NULL;

    rc = sqlite3_prepare_v2(pdb, sql, -1, &stmt, NULL);
    if (stm_unlikely(rc != SQLITE_OK))
        return stm_make_error(err, sqlite3_errcode(pdb), "failed to prepare: %s", sqlite3_errmsg(pdb));
    
    bind_int_by_str(stmt,  srv->port,        ":port");
    bind_text_by_str(stmt, srv->name,        ":name",        true);
    bind_text_by_str(stmt, srv->ip,          ":ip",          true);
    bind_text_by_str(stmt, srv->password,    ":password",    true);
    bind_text_by_str(stmt, srv->description, ":description", true);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return stm_make_error(err, sqlite3_errcode(pdb), "failed to create server (%s): %s",
                                 srv->name, sqlite3_errmsg(pdb));
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

int
libstm_db_server_add(sqlite3 *pdb, libstm_server *srv, libstm_error_t *err)
{
    return add_server(pdb, ADD_SERVER, srv, err);
}

static int
del_server(sqlite3 *pdb, const char *sql, const char *name, libstm_error_t *err)
{
    int rc = 0;
    sqlite3_stmt *stmt = NULL;

    rc = sqlite3_prepare_v2(pdb, sql, -1, &stmt, 0);
    if (stm_unlikely(rc != SQLITE_OK))
        return stm_make_error(err, 0, "failed to prepare statement: `%s`", sqlite3_errmsg(pdb));

    rc = sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    if (stm_unlikely(rc != SQLITE_OK)) {
        sqlite3_finalize(stmt);
        return stm_make_error(err, 0, "failed to bind parameter: `%s`", sqlite3_errmsg(pdb));
    }
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
        return stm_make_error(err, 0, "failed to step: `%s`", sqlite3_errmsg(pdb));

    rc = sqlite3_changes(pdb);
    sqlite3_finalize(stmt);

    if (rc == 0)
        return stm_make_error(err, 0, "the account `%s` does not exists", name);

    fprintf(stdout, "the account `%s`, was deleted\n", name);
    return 0;
}


int
libstm_db_server_del(sqlite3 *pdb, const char *name, libstm_error_t *err)
{
    return del_server(pdb, DELETE_SERVER, name, err);
}



static int
server_get_cb(void *param, int argc, char **argv, char **colname)
{
    libstm_server *srv = (libstm_server *)param;
    if (argc < 0)
        return 0;

    for (int i = 0; i < argc; ++i) {
        switch(hash_string(colname[i])) {
            case NAME:        srv->name        = xstrdup(argv[i]); break;
            case IP:          srv->ip          = xstrdup(argv[i]); break;
            case PORT:        srv->port        = atoi(argv[i]);    break;
            case PASSWORD:    srv->password    = !argv[i] ? NULL : xstrdup(argv[i]); break;
            case DESCRIPTION: srv->description = !argv[i] ? NULL : xstrdup(argv[i]); break;
        }
    }

    return 0;
}

libstm_server *
libstm_db_server_get(sqlite3 *pdb, const char *name, libstm_error_t *err)
{
    int rc = 0;
    char query[BUFSIZ] = {0};
    libstm_server *srv = NULL;

    snprintf(query, BUFSIZ, SELECT_ALL_WHERE_NAME_XXX, name);
    srv = xmalloc(sizeof(libstm_server));

    rc = doquery_with_callback(pdb, query, server_get_cb, srv);
    if (rc != 0) {
        stm_make_error(err, 0, "failed to execute query `%s` (%s)", query, sqlite3_errmsg(pdb));
        return NULL;
    }

    if (stm_unlikely(srv->name == NULL))
        return NULL;

    return srv;
}
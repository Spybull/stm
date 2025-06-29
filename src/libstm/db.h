#ifndef LIBSTM_DB_H
#define LIBSTM_DB_H
#include "error.h"
#include <sqlite3.h>

#define CREATE_DRYDB                                                    \
"                                                                       \
CREATE TABLE [REMOTES] (                                                \
    id          INTEGER PRIMARY KEY,                                    \
    name        TEXT NOT NULL CHECK (name != ''),                       \
    ip          TEXT NOT NULL,                                          \
    port        INTEGER NOT NULL CHECK (port >= 0 AND port <= 65535),   \
    description TEXT,                                                   \
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP);                    \
"

struct libstm_server_s {
	char *name;
	const char *ip;
	unsigned short port;
	char *description;
};
typedef struct libstm_server_s libstm_server;

sqlite3 *libstm_db_open(const char *filename, const char *pKey, libstm_error_t *err);
int libstm_db_init(const char *filename, const char *pKey, int nKey, const char *scheme, libstm_error_t *err);
int libstm_db_decrypt(sqlite3 *pdb, const char *pKey, int nKey, libstm_error_t *err);
int libstm_db_server_add(sqlite3 *pdb, libstm_server *srv, libstm_error_t *err);

/* sqlcipher functions */
extern int sqlite3_key_v2(
  sqlite3 *db,                   /* Database to be keyed */
  const char *zDbName,           /* Name of the database */
  const void *pKey, int nKey     /* The key */
);

extern int sqlite3_rekey_v2(
    sqlite3 *db,               /* Database to be rekeyed */
    const char *zDbName,       /* Which ATTACHed database to rekey */
    const void *pKey, int nKey /* The new key */
);



#endif
#ifndef LIBSTM_DB_H
#define LIBSTM_DB_H
#include "error.h"
#include <sqlite3.h>
#include <stdbool.h>

typedef int (*sqlite_cb)(void *, int, char *[], char *[]);

#define CREATE_DRYDB                                                     	 \
"                                                                        	 \
CREATE TABLE [SERVERS] (                                                 	 \
	id          INTEGER PRIMARY KEY,                                     	 \
	name        TEXT NOT NULL CHECK (name != '') UNIQUE,                 	 \
	ip          TEXT NOT NULL,                                           	 \
	port        INTEGER DEFAULT 22 CHECK (port >= 0 AND port <= 65535),  	 \
	proto       TEXT DEFAULT 'TCP',                                      	 \
	login       TEXT,                                                    	 \
	creds       TEXT,                                                    	 \
	description TEXT,                                                    	 \
	created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,                      	 \
	UNIQUE(ip, port, login, name));                                      	 \
																		 	 \
CREATE TABLE [SNAPSHOTS] (												 	 \
	id 		   INTEGER PRIMARY KEY,										 	 \
    server_id  INTEGER NOT NULL REFERENCES SERVERS(id) ON DELETE CASCADE,	 \
    timestamp  DATETIME DEFAULT CURRENT_TIMESTAMP,						 	 \
    note       TEXT,													 	 \
    UNIQUE(server_id, timestamp));										 	 \
																			 \
CREATE TABLE [SNAP_ENTRIES] (											 	 \
    id          INTEGER PRIMARY KEY,									 	 \
    snapshot_id INTEGER NOT NULL REFERENCES SNAPSHOTS(id) ON DELETE CASCADE, \
    name        TEXT NOT NULL,												 \
    encoding    TEXT DEFAULT 'zstd',										 \
    content     BLOB NOT NULL,												 \
    UNIQUE(snapshot_id, name));												 \
"

#define CREATE_DRYDB_META							\
"													\
CREATE TABLE [SERVERS_META] (						\
	id    INTEGER PRIMARY KEY,						\
	name  TEXT NOT NULL CHECK (name != '') UNIQUE);	\
"

enum
{
	NAME  = 0x2f8b3bf4,
	IP    = 0x687720b2,
	PORT  = 0x5fe28198,
	PROTO = 0x35b2a389,
	LOGIN = 0x45a1edc,
	CREDS = 0xb8b76830,
	DESCRIPTION = 0x6b7119c1
};

struct libstm_server_s {
	char *name;
	char *ip;
	unsigned short port;
	char *proto;
	char *login;
	char *creds;
	char *description;
};
typedef struct libstm_server_s libstm_server;

STM_HIDDEN int libstm_db_create(const char *filename, const char *scheme, libstm_error_t *err);
STM_HIDDEN int libstm_db_init(const char *filename, const char *pKey, int nKey, const char *scheme, libstm_error_t *err);
STM_HIDDEN int libstm_db_decrypt(sqlite3 *pdb, const char *pKey, int nKey, libstm_error_t *err);

STM_API void libstm_server_free(libstm_server *srv);
STM_API sqlite3 *libstm_db_open(const char *filename, const char *pKey, libstm_error_t *err);
STM_API int libstm_db_server_add_metadata(sqlite3 *pdb, libstm_server *srv, libstm_error_t *err);
STM_API int libstm_db_server_add(sqlite3 *pdb, libstm_server *srv, libstm_error_t *err);
STM_API int libstm_db_server_del(sqlite3 *pdb, const char *name, libstm_error_t *err);
STM_API libstm_server *libstm_db_server_get(sqlite3 *pdb, const char *name, libstm_error_t *err);

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
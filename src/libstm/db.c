#include "db.h"
#include "file.h"
#include "utils.h"

int
libstm_db_init(const char *filename, const char *pKey, int nKey, const char *scheme, libstm_error_t *err)
{
    int rc;
    sqlite3 *pdb;

    rc = stm_create_file(filename, err);
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
    rc = sqlite3_key_v2(pdb, "stm.db", pKey, nKey);
    if (rc != SQLITE_OK)
        return stm_make_error(err, 0, "failed to decrypt database");
    return 0;
}

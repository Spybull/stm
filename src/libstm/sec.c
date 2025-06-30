#include "sec.h"
#include "utils.h"
#include "db.h"
#include "config.h"

#include <stdbool.h>
#include <openssl/evp.h>

char *
libstm_ask_password(const char *prompt, int verify, libstm_error_t *err)
{
    int rc;
    char passwd[BUFSIZ - 1] = {0};

    rc = EVP_read_pw_string(passwd, sizeof(passwd), prompt, verify);
    if (rc < 0) {
        stm_make_error(err, errno, "EVP_read_pw_string error");
        return NULL;
    }

    trim(passwd);

    if (!strlen(passwd)) {
        stm_make_error(err, 0, "empty password is not allowed");
        return NULL;
    }

    return xstrdup0(passwd);
}

sqlite3 *
libstm_db_auth(const char *prompt, char *pwout, libstm_error_t *err)
{
    int rc = 0;
    int attempts = 3;
    sqlite3 *pdb = NULL;

    if (!prompt || !strlen(prompt))
        prompt = "Enter decryption key: ";
    
    pdb = libstm_db_open(STM_DATABASE_NAME, NULL, err);
    if (pdb == NULL)
        return NULL;

    do
    {
        if (!attempts) {
            sqlite3_close_v2(pdb);
            stm_make_error(err, 0, "maximum attempts remaining (failed password)");
            return NULL;
        }

        char *passwd = libstm_ask_password(prompt, 0, err);
        if (!passwd)
            return NULL;

        rc = libstm_db_decrypt(pdb, passwd, strlen(passwd), err);
        if (rc == -(SQLITE_NOTADB + 1)) {
            fprintf(stderr, "wrong password, try again"
                            "(attempts remaining: %d)\n", --attempts);
        } else {
            if (pwout != NULL)
                memcpy(pwout, passwd, strlen(passwd));

            memset(passwd, '\0', strlen(passwd));
            free(passwd);
            return pdb;
        }

        memset(passwd, '\0', strlen(passwd));
        free(passwd);
    } while(true);

    return pdb;
}
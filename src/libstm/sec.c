#include "sec.h"
#include "utils.h"
#include "db.h"
#include "config.h"

#include <stdbool.h>
#include <string.h>
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

    char *new_passwd = xstrdup(passwd);
    explicit_bzero(passwd, strlen(passwd));
    return new_passwd;
}

sqlite3 *
libstm_db_auth(const char *prompt, char *pwout, libstm_error_t *err)
{
    int rc = 0;
    int attempts = 3;
    sqlite3 *pdb = NULL;
    smtcred_t creds = { 0 };
    bool wanna_cache = false;

    /* trying to ask credential daemon for password */
    rc = libstm_is_daemon_active(STM_CRED_PID_PATH, err);
    if (stm_unlikely(rc < 0))
        return NULL;

    if (!prompt || !strlen(prompt))
        prompt = "Enter decryption key: ";
    
    pdb = libstm_db_open(STM_DATABASE_NAME, NULL, err);
    if (pdb == NULL)
        return NULL;

    /* if daemon is already active */
    if (rc)
    {
        rc = libstm_is_password_cached(&creds, err);
        if (stm_unlikely(rc < 0))
            return NULL;
        else if (rc == 0) // still not cached
            wanna_cache = true;
        else { // use already cached password
            rc = libstm_db_decrypt(pdb, creds.password, creds.len, err);
            return rc < 0 ? NULL : pdb;
        }
    }

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
            if (wanna_cache) {
                rc = libstm_cache_creds(passwd, err);
                if (rc < 0)
                    return NULL;
            }
            
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

void
libstm_secure_memzero(void *ptr, size_t l)
{
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (l--)
        *p++ = 0;
}
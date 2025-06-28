#include "sec.h"
#include "utils.h"
#include <openssl/evp.h>

char *
stm_ask_password(const char *prompt, int verify, libstm_error_t *err)
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
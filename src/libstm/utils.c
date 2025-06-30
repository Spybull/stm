#include "utils.h"

#include <linux/limits.h>

void
trim(char *line)
{
    char *p, *q;
    int space = 1;

    for(p = q = line; *p; p++) {
        if (isspace((unsigned char)*p)) {
            if (!space)
                *q++ = ' ';

            space = 1;
            continue;
        }
        *q++ = *p;
        space = 0;
    }

    if (q > line && q[-1] == ' ')
        q--;

    *q = '\0';
}

int
libstm_get_workdir(char *out, libstm_error_t *err)
{
    int rc;
    cleanup_free char *home_path = NULL;

    const char *env = getenv("HOME");
    if (stm_likely(env != NULL)) {
        home_path = xstrdup(env);
    } else {
        home_path = getcwd(NULL, 0);
        if (stm_unlikely(home_path == NULL))
            return stm_make_error(err, errno, "failed to getcwd");
    }

    snprintf(out, PATH_MAX, "%s", home_path);

    return 0;
}
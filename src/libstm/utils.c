#include "utils.h"


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
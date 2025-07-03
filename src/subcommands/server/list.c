#include "list.h"
#include <argp.h>

#include "libstm/db.h"
#include "libstm/sec.h"
#include "libstm/config.h"
#include "libstm/utils.h"
#include "libstm/formatter.h"

static unsigned int flags = 0;
static struct argp_option options[] = {
    { "no-headers", 'n', 0, 0, "Skip headers when output is csv", 0},
    { "format", 'f', "FORMAT", 0, "Output format: json or csv (default: \"json\")", 0 },
    { 0, }
};

static error_t
parse_opt(int key, char *arg, struct argp_state *state stm_unused) {

    switch (key)
    {
        case 'n':
            flags |= NOHEADERS;
        break;

        case 'f':
            if (strcmp(arg, "json") == 0) {
                flags |= FORMAT_JSON;
                flags &= ~FORMAT_CSV;
            }
            else if (strcmp(arg, "csv") == 0) {
                flags |= FORMAT_CSV;
            } else
                libstm_fail_with_error(0, "invalid output format");
        break;
        
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char doc[] = "STM server list";
static struct argp argp = { options, parse_opt, NULL, doc, NULL, NULL, NULL };

int
stm_server_subcmd_list(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err stm_unused)
{
    int rc = 0, farg = 0;
    flags |= FORMAT_JSON;
    argp_parse(&argp, argc, argv, 0, &farg, NULL);

    glob_args->pdb = libstm_db_auth(NULL, NULL, err);
    if (!glob_args->pdb)
        return STM_GENERIC_ERROR;
    
    if (CHECK_FLAGS(flags, FORMAT_CSV))
        rc = stmlib_fmt_print_csv(glob_args->pdb, "SELECT * FROM SERVERS;", !CHECK_FLAGS(flags, NOHEADERS), err);
    else
        rc = stmlib_fmt_print_json(glob_args->pdb, "SELECT * FROM SERVERS;", err);

    return rc;
}
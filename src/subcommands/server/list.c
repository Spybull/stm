#include "list.h"
#include <argp.h>

#include "libstm/db.h"
#include "libstm/queries.h"
#include "libstm/sec.h"
#include "libstm/config.h"
#include "libstm/utils.h"
#include "libstm/formatter.h"

static unsigned int flags = 0;
static const char *group = NULL;
static struct argp_option options[] = {
    { "no-headers", 'n', 0, 0, "Skip headers when output is csv", 0},
    { "format", 'f', "FORMAT", 0, "Output format: json or csv (default: \"json\")", 0 },
    { "group", 'g', "STRING", 0, "list servers by group", 0},
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
        
        case 'g':
            group = arg;
        break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char doc[] = "STM server list";
static struct argp argp = { options, parse_opt, NULL, doc, NULL, NULL, NULL };

int
stm_server_subcmd_list(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
{
    int rc = 0, farg = 0;
    flags |= FORMAT_JSON;
    argp_parse(&argp, argc, argv, 0, &farg, NULL);

    glob_args->pdb = libstm_db_auth(NULL, NULL, glob_args->stmd_creds_pid_path, glob_args->stmd_creds_sock_path, err);
    if (!glob_args->pdb)
        return STM_GENERIC_ERROR;
    
    char query[BUFSIZ] = {0};

    if (!group)
        snprintf(query, sizeof(query), "%s", SELECT_ALL_FROM_SERVERS);
    else
        snprintf(query, BUFSIZ, SELECT_ALL_SERVERS_BY_GROUP, group);

    if (CHECK_FLAGS(flags, FORMAT_CSV)) {
        rc = libstm_fmt_print_csv(glob_args->pdb, query, !CHECK_FLAGS(flags, NOHEADERS), err);
    } else {
        rc = libstm_fmt_print_json(glob_args->pdb, query, err);
    }

    return rc;
}
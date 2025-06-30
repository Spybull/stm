#include "list.h"
#include <argp.h>

#include "libstm/db.h"
#include "libstm/config.h"
#include "libstm/utils.h"

static unsigned int flags = 0;
static struct argp_option options[] = {
    { "no-headers", 'n', 0, 0, "Skip headers when output is csv", 0},
    { "format", 'f', "FORMAT", 0, "Output format: json or csv (default: \"csv\")", 0 },
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
            else if (strcmp(arg, "csv") == 0)
                flags |= FORMAT_CSV;
            else
                libstm_fail_with_error(0, "invalid output format");
        break;
        
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char doc[] = "\nSUBCOMMANDS:\n"
                    "\tserver - show server records\n";
static char args_doc[] = "list";
static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

int
stm_command_list(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err stm_unused)
{
    int farg = 0;
    flags |= FORMAT_CSV;
    argp_parse(&argp, argc, argv, 0, &farg, NULL);
    
    if (strcmp(argv[farg], "server")) {

    }

    return 0;
}
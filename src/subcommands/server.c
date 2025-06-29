#include "server.h"
#include <argp.h>

#include "libstm/db.h"
#include "libstm/utils.h"
#include "libstm/sec.h"

enum {
    SERVER_IPADDR = 1001,
    SERVER_PORT,
    SERVER_DESCRIPTION
};

static struct argp_option options[] = {
    { "ip",            SERVER_IPADDR,      "STRING", 0,                   "IPv4/IPv6 address", 0},
    { "port",          SERVER_PORT,        "NUMBER", 0,                   "TCP port number",   0},
    { "description",   SERVER_DESCRIPTION, "STRING", OPTION_ARG_OPTIONAL, "description",       0},
    { 0, }
};

static error_t
parse_opt(int key, char *arg, struct argp_state *state) {

    libstm_server *args = state->input;

    switch (key)
    {
        case SERVER_IPADDR: args->ip = arg; break;
        case SERVER_PORT: args->port = (unsigned short)atoi(arg); break;
        case ARGP_KEY_ARG:
            if (state->arg_num == 0) {
                args->name = arg;
            }
        break;
        case ARGP_KEY_NO_ARGS:
            libstm_fail_with_error(0, "please specify server name");
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}
static char doc[] = "server";
static char args_doc[] = "name";
static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

int
stm_subcommand_server(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err)
{
    int rc = 0;
    libstm_server server = {0};
    argp_parse(&argp, argc, argv, 0, NULL, &server);

    if (!server.ip || !server.port)
        return stm_make_error(err, 0, "not all required parameters are specified");

    rc = libstm_auth(NULL, NULL, err);
    if (rc < 0)
        return rc;
    
    printf("Result: %s -> `%s:%d`\n", server.name, server.ip, server.port);
    return 0;
}
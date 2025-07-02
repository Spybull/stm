#include "add.h"
#include <argp.h>
#include <stdbool.h>
#include <string.h>

#include "libstm/utils.h"
#include "libstm/db.h"
#include "libstm/sec.h"
#include "server.h"

static bool interactive = false;
enum {
    SERVER_IPADDR = 1001,
    SERVER_PASS,
    SERVER_PORT = 'p',
    SERVER_DESCRIPTION = 'd'
};

static struct argp_option options[] = {
    { "ip",            SERVER_IPADDR,      "STRING", 0,                   "IPv4/IPv6 address", 0},
    { "port",          SERVER_PORT,        "NUMBER", 0,                   "TCP port number",   0},
    { "password",      SERVER_PASS,        "STRING", OPTION_ARG_OPTIONAL, "Password",          0},
    { "description",   SERVER_DESCRIPTION, "STRING", OPTION_ARG_OPTIONAL, "description",       0},
    { "interactive",   'i',                0,         0,                  "enter password interactively" , 0},
    { 0, }
};

static error_t
parse_opt(int key, char *arg, struct argp_state *state) {

    libstm_server *args = state->input;

    switch (key)
    {
        case SERVER_IPADDR: args->ip = arg; break;
        case SERVER_PORT: args->port = (unsigned short)atoi(arg); break;

        case SERVER_PASS:
            args->password = xstrdup(arg);
            /* TODO: need some sec tests */
            explicit_bzero(arg, strlen(arg)); // or libstm_secure_memzero 
        break;

        case 'i': interactive = true; break;
        case ARGP_KEY_ARG:
            if (state->arg_num == 0)
                args->name = arg;
            else
                return ARGP_ERR_UNKNOWN;
        break;
        case ARGP_KEY_NO_ARGS:
            libstm_fail_with_error(0, "please specify server name");
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char doc[] = "STM server add";
static struct argp argp = { options, parse_opt, "NAME", doc, NULL, NULL, NULL };

int
stm_server_subcmd_add(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err)
{
    libstm_server server = {0};
    argp_parse(&argp, argc, argv, 0, 0, &server);

    if (!server.ip || !server.port)
        return stm_make_error(err, 0, "not all required parameters are specified");

    if (interactive && !server.password) {
        server.password = libstm_ask_password("Enter server password: ", 0, err);
        if (stm_unlikely(server.password == NULL))
            return STM_GENERIC_ERROR;
    }

    glob_args->pdb = libstm_db_auth(NULL, NULL, err);
    if (!glob_args->pdb)
        return STM_GENERIC_ERROR;

    if (!server.password)
        free(server.password);
    return libstm_db_server_add(glob_args->pdb, &server, err);
}
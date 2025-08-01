#include "add.h"
#include <argp.h>
#include <stdbool.h>
#include <string.h>

#include "libstm/utils.h"
#include "libstm/db.h"
#include "libstm/sec.h"
#include "libstm/config.h"
#include "server.h"

static bool interactive = false;
enum {
    SERVER_ADDR  = 'a',
    SERVER_CREDS = 'c',
    SERVER_PORT  = 'p',
    SERVER_PROTO = 'k',
    SERVER_LOGIN = 'l',
    SERVER_DESCRIPTION = 'd',
    SERVER_GROUP = 'g',
    INTERACTIVE_INPUT = 1001
};

static struct argp_option options[] = {
    { "address",       SERVER_ADDR,        "STRING", 0, "IP/HTTP addresses",             0},
    { "port",          SERVER_PORT,        "NUMBER", 0, "TCP port number (default: 22)", 0},
    { "creds",         SERVER_CREDS,       "STRING", 0, "Credentials",                   0},
    { "protocol",      SERVER_PROTO,       "STRING", 0, "Protocol (default: tcp)",       0},
    { "login",         SERVER_LOGIN,       "STRING", 0, "Login (default: root)",         0},
    { "description",   SERVER_DESCRIPTION, "STRING", 0, "Description",                   0},
    { "interactive",   INTERACTIVE_INPUT,   0,       0, "Enter creds interactively",     0},
    { "group",         SERVER_GROUP,       "STRING", 0, "Server group name",             0},
    { 0, }
};

static error_t
parse_opt(int key, char *arg, struct argp_state *state) {

    libstm_server *args = state->input;

    switch (key)
    {
        case SERVER_ADDR:
            args->addr = arg;
        break;

        case SERVER_PORT:
            args->port = (unsigned short)atoi(arg);
        break;

        case SERVER_CREDS:
            args->creds = arg;
        break;

        case SERVER_PROTO:
            args->proto = arg;
        break;

        case SERVER_LOGIN:
            args->login = arg;
        break;

        case INTERACTIVE_INPUT:
            interactive = true;
        break;

        case SERVER_DESCRIPTION:
            args->description = arg;
        break;

        case SERVER_GROUP:
            args->group = arg;
        break;

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
    int rc = 0;
    libstm_server server = {
        .port = 22,
        .proto = "tcp",
        .login = "root",
        .group = "main"
    };

    argp_parse(&argp, argc, argv, 0, 0, &server);

    if (!server.addr)
        return stm_make_error(err, 0, "address is required");

    if (interactive && !server.creds) {
        server.creds = libstm_ask_password("Enter server credentials: ", 0, err);
        if (stm_unlikely(server.creds == NULL))
            return STM_GENERIC_ERROR;
    }

    glob_args->pdb = libstm_db_auth(NULL, NULL, glob_args->stmd_creds_pid_path, glob_args->stmd_creds_sock_path, err);
    if (!glob_args->pdb)
        return STM_GENERIC_ERROR;

    if (!server.creds)
        free(server.creds);
    
    glob_args->mpdb = libstm_db_open(STM_DATABASE_META, NULL, err);
    if (stm_unlikely(glob_args->mpdb == NULL))
        return STM_GENERIC_ERROR;

    rc = libstm_db_server_add(glob_args->pdb, &server, err);
    if (rc < 0)
        return  STM_GENERIC_ERROR;
    rc = libstm_db_server_add_metadata(glob_args->mpdb, &server, err);
    if (rc < 0)
        return  STM_GENERIC_ERROR;

    fprintf(stdout, "new account `%s` was successfully created\n", server.name);
    return 0;
}
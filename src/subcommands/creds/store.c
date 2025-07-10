#include "store.h"
#include <argp.h>
#include <fcntl.h>
#include <syslog.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <time.h>
#include <sys/select.h>
#include <string.h>
#include <openssl/evp.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/prctl.h>

#include "libstm/file.h"
#include "libstm/utils.h"
#include "libstm/config.h"
#include "libstm/sec.h"


#define MIN_STORE_MINS 1
#define MAX_STORE_MINS 1440
#define DEF_STORE_MINS 60

static const char *cd_name = "stmcd";
static bool save_immediately = false;
static unsigned long timeout = DEF_STORE_MINS;
static char *globcred = NULL;
static void *accept_client(void *);


static void
setup_cred(char **cred, char unsafe_creds[]) {
    if (*cred != NULL)
        free(*cred);

    *cred = xstrdup0s(unsafe_creds);
}

static void
destroy_cred(char **cred) {
    if (cred && *cred) {
        explicit_bzero(*cred, strlen(*cred));
        free(*cred);
        *cred = NULL;
    }
}

static void
handle_signal(int signal)
{
    if (signal == SIGTERM)
        syslog(UINF, "terminating %s", cd_name);
    
    if (signal == SIGALRM)
        syslog(UINF, "terminating %s by timeout", cd_name);

    destroy_cred(&globcred);
    exit(EXIT_SUCCESS);
}

static struct argp_option options[] = {
    { "now",     'n', 0,         0, "save creds immediately", 0 },
    { "timeout", 't', "MINUTES", 0, "daemon live time",       0 },
    { 0, }
};
static error_t
parse_opt(int key, char *arg stm_unused, struct argp_state *state stm_unused) {

    switch (key)
    {
        case 'n':
            save_immediately = true;
        break;

        case 't':
            char *endptr;
            timeout = strtoul(arg, &endptr, 10);
            if (*endptr != '\0')
                return ARGP_ERR_UNKNOWN;

            if (timeout > MAX_STORE_MINS)
                timeout = MAX_STORE_MINS;
            else if (timeout < MIN_STORE_MINS)
                timeout = MIN_STORE_MINS;
        break;

        case ARGP_KEY_ARG:
            return ARGP_ERR_UNKNOWN;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char doc[] = "STM creds store";
static struct argp argp = { options, parse_opt, "NAME", doc, NULL, NULL, NULL };
int
stm_creds_subcmd_store(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
{
    int rc = 0;

    rc = libstm_create_dir(glob_args->xdg_runtime_path, 0700, err);
    if (rc < 0)
        return rc;

    if (chdir(glob_args->xdg_runtime_path) < 0)
        stm_make_error(err, errno, "failed to change directory `%s` ", glob_args->xdg_runtime_path);
    
    argp_parse(&argp, argc, argv, 0, 0, 0);

    rc = libstm_is_daemon_active(STMD_CRED_PID_FILE, err);
    if (rc > 0) {
        fprintf(stdout, "%s already running\n", cd_name);
        return 0;
    }

    if (save_immediately) {
        char buf[BUFSIZ] = {0};
        if (EVP_read_pw_string(buf, sizeof(buf), "enter database password: ", false) < 0)
            return stm_make_error(err, errno, "EVP_read_pw_string error");

        setup_cred(&globcred, buf);
        if (!globcred)
            fprintf(stderr, "could not auth immediately (%s)", (*err)->msg);
    }

    rc = libstm_daemonize(cd_name, err);
    if (rc == PARENT_RC) {
        destroy_cred(&globcred);
        return 0;
    }

    if (rc <= 0) {
        destroy_cred(&globcred);
        return rc;
    }

    for (int i = 0; i < argc; ++i)
        memset(argv[i], 0, strlen(argv[i]));

    size_t len = 0;
    for (int i = 0; i < glob_args->argc; ++i)
        len += strlen(glob_args->argv[i]) + 1;

    strncpy(glob_args->argv[0], cd_name, len - 1);
    memset(glob_args->argv[0] + strlen(cd_name), 0, len - strlen(cd_name));

    syslog(UINF, "starting %s daemon", cd_name);
    if (chdir(glob_args->xdg_runtime_path) < 0) {
        syslog(UERR, "failed to change directory from daemon");
        closelog();
        return STM_GENERIC_ERROR;
    }
    
    int sd = -1; 
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    rc = libstm_create_file(STMD_CRED_PID_FILE, 0600, err);
    if (rc < 0) {
        syslog(UERR, "failed to create pid file (%s)", (*err)->msg);
        return -1;
    }

    int fd_lock = open(STMD_CRED_PID_FILE, O_WRONLY);
    char buf[16];
    sprintf(buf, "%ld", (long)getpid());
    if (write(fd_lock, buf, strlen(buf) + 1) < 0) {
        syslog(UERR, "failed to start daemon: %s", (*err)->msg);
        closelog();
        exit(EXIT_FAILURE);
    }

    libstm_lock_file(fd_lock);
    
    sd = libstm_unix_stream_listen(STMD_CRED_SOCK_FILE, err);
    if (sd < 0) {
        syslog(UERR, "failed to bind `%s` unix socket", STMD_CRED_SOCK_FILE);
        closelog();
        exit(EXIT_FAILURE);
    }

    alarm(60 * timeout);

    while(1)
    {
        int client_fd = accept(sd, NULL, NULL);
        if (client_fd < 0) {
            syslog(UERR, "failed to accept conection: `%s`", strerror(errno));
            continue;
        }

        pthread_t th;
        rc = pthread_create(&th, NULL, accept_client, &client_fd);
        if (rc != 0)
            return rc;

        pthread_detach(th);
    }

    return 0;
}

static void *
accept_client(void *data) {

    cleanup_file FILE *in = NULL;
    cleanup_file FILE *out = NULL;
    
    int sd = *((int *) data);
    int sd2;
    
    sd2 = dup(sd);
    in  = xfdopen(sd, "r");
    out = xfdopen(sd2, "w");

    char buffer[4096] = {0};
    setvbuf(out, NULL, _IONBF, 0);
    setvbuf(in, NULL, _IONBF, 0);
    while (fgets(buffer, 4096, in) != NULL) {

        if (strcmp(buffer, "getcred\n") == 0) {
            syslog(UINF, "get command issued");
            fprintf(out, "%s\n", globcred ? globcred : "");
            fflush(out);
        } else if (strcmp(buffer, "setcred\n") == 0) {
            syslog(UINF, "set command issued");
                char pass[256];
                if (fgets(pass, 256, in) == NULL)
                    return NULL;

                pass[strcspn(pass, "\n")] = 0;
                setup_cred(&globcred, pass);
        } else if (strcmp(buffer, "gettime\n") == 0) {
            syslog(UINF, "time command issued");
            unsigned int remaining_time = alarm(0);
            alarm(remaining_time);
            fprintf(out, "%d\n", remaining_time);
            fflush(out);
        }
    }

    return NULL;
}

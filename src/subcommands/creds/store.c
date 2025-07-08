#include "store.h"
#include <argp.h>
#include <fcntl.h>
#include <syslog.h>
#include <pthread.h>
#include "libstm/file.h"
#include "libstm/utils.h"
#include "libstm/sec.h"
#include <signal.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <time.h>
#include <sys/select.h>
#include <string.h>
#include <openssl/evp.h>

static bool save_immediately = false;
static char *globcred = NULL;
static void *accept_client(void *);

static void
handle_signal(int signal)
{
    if (signal == SIGTERM) {
        syslog(UINF, "terminating %s credential daemon", "STM CRED HELPER");
    } else if (signal == SIGALRM) {
        syslog(UINF, "terminating %s by timeout", "STM CRED HELPER");
        if (globcred)
            explicit_bzero(globcred, strlen(globcred));
        
        free(globcred);
    }

    exit(EXIT_SUCCESS);
}

static void
sigterm_handler(int signo stm_unused)
{
    if (globcred)
        explicit_bzero(globcred, strlen(globcred));
}

static struct argp_option options[] = {
    { "now", 'n', 0, 0, "save creds immediately", 0},
    { 0, }
};
static error_t
parse_opt(int key, char *arg stm_unused, struct argp_state *state stm_unused) {

    switch (key)
    {
        case 'n':
            save_immediately = true;
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
stm_creds_subcmd_store(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err stm_unused)
{
    int rc = 0;
    const char *pid_path = STM_CRED_PID_PATH;
    const char *log_path = STM_CRED_LOG_PATH;

    if (libstm_is_daemon_active(pid_path, err) > 0) {
        fprintf(stdout, "stm credential daemon already running\n");
        return 0;
    }

    argp_parse(&argp, argc, argv, 0, 0, 0);

    if (save_immediately)
    {
        char buf[BUFSIZ] = {0};
        if (EVP_read_pw_string(buf, sizeof(buf), "enter database password: ", false) < 0)
            return stm_make_error(err, errno, "EVP_read_pw_string error");

        globcred = xstrdup0(buf);
        explicit_bzero(buf, sizeof(buf));

        if (!globcred)
            fprintf(stderr, "could not auth immediately (%s)", (*err)->msg);
    }

    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    signal(SIGTERM, sigterm_handler);

    rc = libstm_daemonize(pid_path, log_path, "STM CRED HELPER", err);
    if (rc == PARENT_RC) {
        if (globcred)
            explicit_bzero(globcred, strlen(globcred));
        
        free(globcred);
        return 0;
    }

    if (rc <= 0) {
        syslog(UERR, "failed to start daemon: %s", (*err)->msg);
        closelog();
        exit(EXIT_FAILURE);
    }

    syslog(UINF, "starting stm credential store daemon");
    int fd_lock = open(pid_path, O_WRONLY);
    char buf[16];
    sprintf(buf, "%ld", (long)getpid());
    if (write(fd_lock, buf, strlen(buf) + 1) < 0) {
        syslog(UERR, "failed to start daemon: %s", (*err)->msg);
        closelog();
        exit(EXIT_FAILURE);
    }

    libstm_lock_file(fd_lock);

    const char *unix_socket_path = STM_CRED_SOCK_PATH;
    int sd = libstm_unix_stream_listen(unix_socket_path, err);
    if (sd < 0) {
        syslog(UERR, "failed to bind `%s` unix socket", unix_socket_path);
        closelog();
        exit(EXIT_FAILURE);
    }

    alarm(60 * 60); //60 mins

    while(1)
    {
        int client_fd = accept(sd, NULL, NULL);
        if (client_fd < 0) {
            syslog(UERR, "failed to accept conection: `%s`", strerror(errno));
            continue;
        }

        syslog(UINF, "accepted connection");
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
            syslog(UINF, "getting credentials command issued");
            fprintf(out, "%s\n", globcred ? globcred : "");
            fflush(out);
            
        } else if (strcmp(buffer, "setcred\n") == 0) {
            syslog(UINF, "setting credentials command issued");
                char pass[256];
                if (fgets(pass, 256, in) == NULL)
                    return NULL;

                pass[strcspn(pass, "\n")] = 0;
                globcred = xstrdup0(pass);
                explicit_bzero(pass, strlen(pass));
        } else if (strcmp(buffer, "gettime\n") == 0) {
            unsigned int remaining_time = alarm(0);
            alarm(remaining_time);
            fprintf(out, "%d\n", remaining_time);
            fflush(out);
        }
    }

    return NULL;
}

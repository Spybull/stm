#include "utils.h"
#include "file.h"
#include <stdlib.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <syslog.h>

#include <sys/socket.h>
#include <sys/un.h>

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

int
libstm_daemonize(const char *pid_path, const char *log_path, const char *logname, libstm_error_t *err)
{
    int rc;
    openlog(logname, LOG_PID, LOG_USER);

    /* check .pid file exists for locks */
    rc = libstm_path_exists(pid_path, err);
    if (rc < 0)
        return rc;

    if (rc == 0) {
        if (libstm_create_file(pid_path, 0666, err) < 0 )
            libstm_fail_with_error(errno, "failed to create %s", pid_path);
    }

    rc = libstm_path_exists(log_path, err);
    if (rc < 0) return rc;

    if ( rc == 0 )
        if (libstm_create_file(log_path, 0666, err) < 0 )
            libstm_fail_with_error(errno, "failed to create %s", log_path);

    int fd, maxfd;
    
    pid_t np = fork();
    if (stm_likely(np > 0))
        return 100;
    else if (stm_unlikely(np == -1))
        return -1;

    if (stm_unlikely(setsid() == -1))
        return -2;

    np = fork();
    if (stm_likely(np > 0))
        exit(EXIT_SUCCESS);
    else if (stm_unlikely(np == -1))
        return -3;

    umask(0);
    if ( chdir("/") < 0 )
        return -4;

    maxfd = sysconf(_SC_OPEN_MAX);
    for(int fd = 0; fd < maxfd; ++fd)
        close(fd);

    fd = open("/dev/null", O_RDWR);
    if (stm_unlikely(fd < 0))
        return -5;

    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    /* in daemon */
    pid_t pid = getpid();
    cleanup_close int lock_fd = -1;

    lock_fd = open(pid_path, O_WRONLY);
    if (stm_unlikely(lock_fd < 0)) {
        syslog(LOG_USER | LOG_ERR, "failed to open file `%s`: `%s`", pid_path, strerror(errno));
        return -6;
    }

    rc = libstm_is_file_locked(lock_fd, err);
    if (stm_unlikely(rc < 0))
        return rc;
    else if (rc > 0)
        return stm_make_error(err, 0, "already running, see PID in `%s`", pid_path);

    return pid;
}

int
libstm_unix_stream_listen(const char *path, libstm_error_t *err) {

    int sd = -1;
    struct sockaddr_un sa;

    unlink(path);
    int size = strlen(path) + 1;

    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    memcpy(sa.sun_path, path, size);

    sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (stm_unlikely(sd < 0))
        return stm_make_error(err, errno, "failed to create (UNIX) socket");

    if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
        return -1;

    chmod(path, S_IRUSR|S_IWUSR);

    if (listen(sd, 1) < 0)
        return stm_make_error(err, errno, "failed to listen (UNIX) socket");

    return sd;
}
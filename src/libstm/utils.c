#include "utils.h"
#include "file.h"
#include <stdlib.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <syslog.h>

#include <sys/socket.h>
#include <sys/un.h>

FILE *
xfdopen(int fd, const char *mode) {

    FILE *stream = fdopen(fd, mode); 
    if (stm_unlikely(stream == NULL))
        stm_oom();
    return stream;
}

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
libstm_is_daemon_active(const char *pid_path, libstm_error_t *err)
{
    cleanup_close int fd = -1;

    fd = open(pid_path, O_RDONLY);
    if (stm_unlikely(fd < 0)) {
        if (errno == ENOENT)
            return 0;
        return stm_make_error(err, errno, "failed to open daemon PID file `%s`", pid_path);
    }
    return libstm_is_file_locked(fd, err);
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

int
libstm_unix_stream_connect(const char *path, libstm_error_t *err)
{
	int sd = -1, saved_errno;
	struct sockaddr_un sa;
    int size = strlen(path) + 1;

    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    memcpy(sa.sun_path, path, size);

	sd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sd < 0)
		return stm_make_error(err, errno, "failed to create (UNIX) socket");

    chmod(path, S_IRUSR|S_IWUSR);

	if (connect(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
		goto fail;

	return sd;

fail:
	saved_errno = errno;
	if (sd != -1)
		close(sd);
	errno = saved_errno;
	return -1;

}

int
libstm_is_password_cached(smtcred_t *creds, libstm_error_t *err)
{
    char un_path[108];
    cleanup_close int sd = -1;
    cleanup_close int sd2 = -1;
    cleanup_file FILE *in = NULL;
    cleanup_file FILE *out = NULL;

    sd = libstm_unix_stream_connect(STM_CRED_SOCK_PATH, err);
    if (stm_unlikely(sd < 0))
        return stm_make_error(err, errno, "failed to connect to unix socket `%s`", un_path);

    sd2 = dup(sd);
    if (stm_unlikely(sd2 < 0))
        return stm_make_error(err, errno, "failed to duplicate descriptor");
    
    in  = xfdopen(sd, "r");
    out = xfdopen(sd2, "w");

    fprintf(out, "getcred\n");
    fflush(out);

    char password[256];
    if ( fgets(password, sizeof(password), in) == NULL )
        return 0;

    trim(password);

    if (!strlen(password)) /* the password hasn't been cached yet */
        return 0;

    creds->len = strlen(password);
    creds->password = xstrdup0(password);
    creds->exp = 0;
    return 1;

}

int
libstm_cache_creds(const char *password, libstm_error_t *err)
{
    char un_path[108];
    cleanup_close int sd = -1;
    cleanup_file FILE *out = NULL;

    sd = libstm_unix_stream_connect(STM_CRED_SOCK_PATH, err);
    if (stm_unlikely(sd < 0))
        return stm_make_error(err, errno, "failed to connect to unix socket `%s`", un_path);

    /* trying to set daemon password */
    out = xfdopen(sd, "w");
    fprintf(out, "setcred\n");
    fprintf(out, "%s\n", password);
    fflush(out);
    return 0;

}
#include "utils.h"
#include "file.h"
#include <stdlib.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <syslog.h>
#include <pwd.h>
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
libstm_daemonize(const char *logname, libstm_error_t *err)
{
    openlog(logname, LOG_PID, LOG_USER);
    int fd, maxfd;
    
    pid_t np = fork();
    if (stm_likely(np > 0))
        return PARENT_RC;
    else if (stm_unlikely(np == -1)) {
        syslog(UERR, "failed in first fork)");
        return stm_make_error(err, errno, "could not create daemon process");
    }

    if (stm_unlikely(setsid() == -1)) {
        syslog(UERR, "failed to setsid()");
        return STM_GENERIC_ERROR;
    }

    np = fork();
    if (stm_likely(np > 0))
        return PARENT_RC;
    else if (stm_unlikely(np == -1)) {
        syslog(UERR, "failed in second fork)");
        return STM_GENERIC_ERROR;
    }

    umask(0);
    if ( chdir("/") < 0 ) {
        syslog(UERR, "failed to chdir in daemon process)");
        return STM_GENERIC_ERROR;
    }

    maxfd = sysconf(_SC_OPEN_MAX);
    for(int fd = 0; fd < maxfd; ++fd)
        close(fd);

    fd = open("/dev/null", O_RDWR);
    if (stm_unlikely(fd < 0)) {
        syslog(UERR, "failed to open /dev/null in daemon process)");
        return STM_GENERIC_ERROR;
    }

    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    return getpid();
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
libstm_is_password_cached(smtcred_t *creds, const char *daemon_socket, libstm_error_t *err)
{
    cleanup_close int sd = -1;
    cleanup_close int sd2 = -1;
    cleanup_file FILE *in = NULL;
    cleanup_file FILE *out = NULL;

    sd = libstm_unix_stream_connect(daemon_socket, err);
    if (stm_unlikely(sd < 0))
        return stm_make_error(err, errno, "failed to connect to unix socket");

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

    explicit_bzero(password, strlen(password));
    return 1;

}

int
libstm_unix_stream_get_rtime(const char *sock_path, libstm_error_t *err)
{
    cleanup_close int sd = -1;
    cleanup_close int sd2 = -1;
    cleanup_file FILE *in = NULL;
    cleanup_file FILE *out = NULL;

    sd = libstm_unix_stream_connect(sock_path, err);
    if (stm_unlikely(sd < 0))
        return stm_make_error(err, errno, "failed to connect to unix socket");

    sd2 = dup(sd);
    if (stm_unlikely(sd2 < 0))
        return stm_make_error(err, errno, "failed to duplicate descriptor");
    
    in  = xfdopen(sd, "r");
    out = xfdopen(sd2, "w");

    fprintf(out, "gettime\n");
    fflush(out);

    unsigned int rtime = 0;
    if (fscanf(in, "%u", &rtime) != 1)
        return stm_make_error(err, 0, "failed to parse response");

    return rtime;
}

int
libstm_cache_creds(const char *password, const char *daemon_socket, libstm_error_t *err)
{
    char un_path[108];
    cleanup_close int sd = -1;
    cleanup_file FILE *out = NULL;

    sd = libstm_unix_stream_connect(daemon_socket, err);
    if (stm_unlikely(sd < 0))
        return stm_make_error(err, errno, "failed to connect to unix socket `%s`", un_path);

    /* trying to set daemon password */
    out = xfdopen(sd, "w");
    setvbuf(out, NULL, _IONBF, 0);

    fprintf(out, "setcred\n");
    fprintf(out, "%s\n", password);
    fflush(out);
    return 0;

}

char *
whoami(libstm_error_t *err) {
    
    struct passwd *pw;
    uid_t NO_UID = -1;
    uid_t uid;

    errno = 0;
    uid = geteuid();
    
    pw = uid == NO_UID && errno ? NULL : getpwuid(uid);
    
    if (!pw) {
        stm_make_error(err, errno, "cannot find name for user ID %lu", (unsigned long int) uid);
        return NULL;
    }
    
    return xstrdup(pw->pw_name);
}

pid_t
read_pid_file(const char *lock_file, libstm_error_t *err) {
    
    char buf[16] = {0};
    cleanup_close int fd = 0;

    fd = open(lock_file, O_RDONLY);
    if (fd < 0) {
        if (errno == ENOENT) {
            return stm_make_error(err, 0, "stm creds daemon is not running");
        }

        return stm_make_error(err, errno, "failed to open %s", lock_file);
    }

    ssize_t nr = 0;
    if ( ( nr = read(fd, buf, sizeof(buf))) < 0 )
        return stm_make_error(err, errno, "failed to read %s", lock_file);

    if (nr == 0)
        return stm_make_error(err, 0, "no such process pid in %s", lock_file);

    return (pid_t)atoi(buf);
}
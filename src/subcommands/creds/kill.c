#include "kill.h"
#include <argp.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "libstm/utils.h"
#include "libstm/config.h"

static char doc[] = "STM creds kill daemon";
static struct argp argp = { 0, NULL, NULL, doc, NULL, NULL, NULL };
int
stm_creds_subcmd_kill(stm_glob_args *glob_args, int argc, char **argv, libstm_error_t *err)
{
    int rc = 0;
    argp_parse(&argp, argc, argv, 0, 0, 0);

    rc = libstm_is_daemon_active(glob_args->stmd_creds_pid_path, err);
    if (rc < 0)
        return STM_GENERIC_ERROR;

    if (rc > 0) {
        pid_t pid = read_pid_file(glob_args->stmd_creds_pid_path, err);
        if (pid < 0)
            return pid;

        rc = kill(pid, SIGTERM);
        if (rc < 0)
            return stm_make_error(err, errno, "failed to kill daemon by pid `%d`", pid);

        fprintf(stdout, "stm credential daemon was killed (PID: %d)\n", pid);
        unlink(glob_args->stmd_creds_pid_path);
        unlink(glob_args->stmd_creds_sock_path);
        return 0;
    }
    
    fprintf(stdout, "stm credential daemon is inactive\n");
    return 0;
}
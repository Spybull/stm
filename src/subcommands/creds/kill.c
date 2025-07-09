#include "kill.h"
#include <argp.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "libstm/utils.h"

static char doc[] = "STM creds kill daemon";
static struct argp argp = { 0, NULL, NULL, doc, NULL, NULL, NULL };
int
stm_creds_subcmd_kill(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err stm_unused)
{
    int rc = 0;
    argp_parse(&argp, argc, argv, 0, 0, 0);

    rc = libstm_is_daemon_active(STM_CRED_PID_PATH, err);
    if (rc < 0)
        return STM_GENERIC_ERROR;

    if (rc > 0) {
        pid_t pid = read_pid_file(STM_CRED_PID_PATH, err);
        if (pid < 0)
            return pid;

        rc = kill(pid, SIGTERM);
        if (rc < 0)
            return stm_make_error(err, errno, "failed to kill daemon by pid `%d`", pid);

        fprintf(stdout, "stm credential daemon was killed (PID: %d)\n", pid);
        unlink(STM_CRED_PID_PATH);
        unlink(STM_CRED_SOCK_PATH);
        return 0;
    }
    
    fprintf(stdout, "stm credential daemon is inactive\n");
    return 0;
}
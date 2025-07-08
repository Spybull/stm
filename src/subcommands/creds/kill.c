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
    argp_parse(&argp, argc, argv, 0, 0, 0);

    if (libstm_is_daemon_active(STM_CRED_PID_PATH, err) > 0) {
        // if it's active
        pid_t pid = read_pid_file(STM_CRED_PID_PATH, err);
        if (pid < 0)
            return pid;

        if (kill(pid, SIGTERM) == -1)
            return stm_make_error(err, errno, "failed to kill daemon by pid `%d`", pid);

        fprintf(stdout, "stm credential daemon was killed\n");
        unlink(STM_CRED_PID_PATH);
        unlink(STM_CRED_SOCK_PATH);
        return 0;
    }
    
    fprintf(stdout, "stm credential daemon is inactive\n");
    return 0;
}
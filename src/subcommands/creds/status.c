#include "status.h"
#include <argp.h>
#include "libstm/utils.h"

static char doc[] = "STM creds status";
static struct argp argp = { 0, NULL, NULL, doc, NULL, NULL, NULL };
int
stm_creds_subcmd_status(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err stm_unused)
{
    int rc = 0, rtime = 0;
    argp_parse(&argp, argc, argv, 0, 0, 0);

    if (libstm_is_daemon_active(STM_CRED_PID_PATH, err) > 0) {
        // if it's active
        rtime = libstm_unix_stream_get_rtime(err);
        if (rtime < 0)
            return rtime;

        char time_str[32];
        unsigned int hours = rtime / 3600;
        unsigned int minutes = (rtime % 3600) / 60;
        unsigned int seconds = rtime % 60;
        snprintf(time_str, sizeof(time_str), "%02u:%02u:%02u", hours, minutes, seconds);
        
        rc = read_pid_file(STM_CRED_PID_PATH, err);
        if (rc < 0)
            return rc;

        fprintf(stdout, "Daemon is active (PID: %d). Remaining time: %s\n", rc, time_str);
        return 0;
    }
    
    fprintf(stdout, "stm credential daemon is inactive\n");
    return 0;
}
#ifndef STM_SYS_USER
#define STM_SYS_USER
#include <pwd.h>

struct user_info_s {
    uid_t uid, euid;
    gid_t gid, egid;
    char *user_name;
    char *shell;
    char *home_dir;
};
typedef struct user_info_s user_info_t;

user_info_t *libstm_setup_userinfo();
void free_userinfo(user_info_t *user);
void print_userinfo(user_info_t *user);
#endif
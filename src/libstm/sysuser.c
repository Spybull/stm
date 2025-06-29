#include "sysuser.h"
#include "utils.h"

user_info_t *
libstm_setup_userinfo() {

    user_info_t *user = xmalloc(sizeof(user_info_t));
    const char *sudo_user = getenv("SUDO_USER");

    if (sudo_user != NULL) {
        
        struct passwd *pw = getpwnam(sudo_user);
        if (stm_unlikely(pw == NULL))
            exit(EXIT_FAILURE);
        
        user->euid = user->uid = pw->pw_uid;
        user->egid = user->gid = pw->pw_gid;
        user->user_name = xstrdup(sudo_user);
        user->home_dir = xstrdup(pw->pw_dir);
        user->shell = xstrdup(pw->pw_shell);
        return user;
    }

    struct passwd *entry;
    uid_t uid, euid;
    gid_t gid, egid;

    uid   = getuid();  gid  = getgid();
    euid  = geteuid(); egid = getegid();
    entry = getpwuid(uid);

    if (entry) {
        user->user_name = xstrdup(entry->pw_name);
        user->shell = (entry->pw_shell && entry->pw_shell[0])
                                ? xstrdup (entry->pw_shell)
                                : xstrdup ("/bin/sh");
        user->home_dir = xstrdup(entry->pw_dir);
    } else {
        user->user_name = xstrdup("I have no name!");
        user->shell     = xstrdup("/bin/sh");
        user->home_dir  = xstrdup("/");
    }

    user->uid = uid; user->euid = euid;
    user->gid = gid; user->egid = egid;
    return user;
}

void
free_userinfo(user_info_t *user) {
    free(user->user_name);
    free(user->shell);
    free(user->home_dir);
}
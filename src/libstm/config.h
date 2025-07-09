#ifndef CONFIG_H
#define CONFIG_H

#define CHECK_FLAGS(value, mask) (((value) & (mask)) == (mask))
#define NOHEADERS   (1 << 1)
#define FORMAT_CSV  (1 << 2)
#define FORMAT_JSON (1 << 3)
#define FORMAT_UNKNOWN (1 << 4)

#ifdef __linux__
    #define STM_DIR_NAME "stm"
    #define STM_DATABASE_NAME "stm.db"
    #define STM_DATABASE_META "stm_meta.db"

    #define STMD_CRED_PID_FILE  "stmd.pid"
    #define STMD_CRED_SOCK_FILE "stmd.sock"
#elif _WIN32
#else
    #error unsupported platform
#endif

#endif
#ifndef CONFIG_H
#define CONFIG_H

#define CHECK_FLAGS(value, mask) (((value) & (mask)) == (mask))
#define NOHEADERS   (1 << 1)
#define FORMAT_CSV  (1 << 2)
#define FORMAT_JSON (1 << 3)
#define FORMAT_UNKNOWN (1 << 4)

#ifdef __linux__
    #define STM_SYSDIR_PATH   "/var/lib/stm"
    #define STM_USER_DIR_NAME ".stm"
    #define STM_DATABASE_NAME "stm.db"
    #define STM_LOG_FILE      "/var/log/stm.log"
#elif _WIN32
#else
    #error unsupported platform
#endif

#endif
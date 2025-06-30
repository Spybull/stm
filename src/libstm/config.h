#ifndef CONFIG_H
#define CONFIG_H

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
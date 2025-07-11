#pragma once

#if defined(_WIN32)
#   ifdef STM_BUILD
#       define STM_API __declspec(dllexport)
#   else
#       define STM_API __declspec(dllimport)
#   endif
#else
#   define STM_VISIBLE __attribute__((visibility("default")))
#   define STM_HIDDEN  __attribute__((visibility("hidden")))
#   define STM_API STM_VISIBLE
#endif

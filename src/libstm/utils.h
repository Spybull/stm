#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef TEMP_FAILURE_RETRY
#  define TEMP_FAILURE_RETRY(expression)      \
    (__extension__ ({                         \
      long int __result;                      \
      do                                      \
        __result = (long int) (expression);   \
      while (__result < 0 && errno == EINTR); \
      __result;                               \
    }))
#endif

#ifdef __GNUC__
    #define stm_inline inline    __attribute__      ((always_inline))
    #define stm_unused           __attribute__      ((unused))
    #define stm_likely(x)        __builtin_expect   (!!(x), 1)
    #define stm_unlikely(x)      __builtin_expect   (!!(x), 0)
    #define stm_prefetch(x, ...) __builtin_prefetch (x, __VA_ARGS__)

    #define stm_oom() do                  \
    {                                      \
        fprintf (stderr, "out of memory"); \
        _exit (EXIT_FAILURE);              \
    } while (0)

#else
    #define stm_inline inline
    #define stm_likely(x)        (x)
    #define stm_unlikely(x)      (x)
    #define stm_prefetch(x, ...) (x, __VA_ARGS__)
#endif

#define cleanup_free __attribute__ ((cleanup (cleanup_freep)))
#define cleanup_file __attribute__ ((cleanup (cleanup_filep)))
#define cleanup_close __attribute__ ((cleanup (cleanup_closep)))

inline static void
cleanup_freep (void *p) {
  void **pp = (void **) p;
  free (*pp);
}

inline static void
cleanup_filep (FILE **f) {
  FILE *file = *f;
  if (file)
    (void) fclose (file);
}

static inline void
cleanup_closep (void *p)
{
  int *pp = (int *) p;
  if (*pp >= 0)
    TEMP_FAILURE_RETRY (close (*pp));
}

__attribute__ ((malloc)) static inline void *
xmalloc (size_t size)
{
  void *rc = malloc(size);
  if (stm_unlikely(rc == NULL))
    stm_oom();
  return rc;
}

#endif
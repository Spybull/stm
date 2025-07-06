#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

#include "error.h"

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

#define cleanup_free      __attribute__ ((cleanup (cleanup_freep)))
#define cleanup_free_zero __attribute__ ((cleanup (cleanup_freep_zero)))
#define cleanup_file __attribute__ ((cleanup (cleanup_filep)))
#define cleanup_close __attribute__ ((cleanup (cleanup_closep)))
#define FNV_OFFSET 2166136261
#define FNV_PRIME 16777619

#define STM_CRED_PID_PATH  "/var/lib/stm/cred.pid"
#define STM_CRED_LOG_PATH  "/var/lib/stm/cred.log"
#define STM_CRED_SOCK_PATH "/var/lib/stm/cred.sock"
#define PARENT_RC 666
struct stmcred_s {
   char *password;
   size_t len;
   int exp;
};
typedef struct stmcred_s smtcred_t;

static inline unsigned int
hash_string(const char *s)
{
  unsigned int i;

	for (i = FNV_OFFSET; *s; s++) {
		i += (i<<1) + (i<<4) + (i<<7) + (i<<8) + (i<<24);
		i ^= *s;
	}

	return i;
}

inline static void
cleanup_freep (void *p) {
  void **pp = (void **) p;
  free (*pp);
}

inline static void
cleanup_freep_zero (void *p) {
  void **pp = (void **) p;
  if (pp && *pp)
    explicit_bzero(*pp, strlen((char *)*pp));
  free(*pp);
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

__attribute__ ((malloc)) static inline void *
xmalloc0 (size_t size)
{
	void *res = calloc (1, size);
	if (stm_unlikely (res == NULL))
		stm_oom();
	return res;	
}

__attribute__ ((malloc)) static inline char *
xstrdup (const char *s) {
  char *_s = strdup(s);
  if (_s == NULL)
    stm_oom();
  return _s;
}

__attribute__ ((malloc)) static inline char *
xstrdup0 (char *s)
{
  char *_s = strdup(s);
  memset(s, 0, strlen(s));
  if (_s == NULL)
    stm_oom();

  return _s;
}

FILE *xfdopen(int fd, const char *mode);


void trim(char *line);
int libstm_get_workdir(char *out, libstm_error_t *err);

int libstm_daemonize(const char *pid_path, const char *log_path, const char *logname, libstm_error_t *err);
int libstm_is_daemon_active(const char *pid_path, libstm_error_t *err);
int libstm_cache_creds(const char *password, libstm_error_t *err);
int libstm_is_password_cached(smtcred_t *creds, libstm_error_t *err);

int libstm_unix_stream_listen(const char *path, libstm_error_t *err);
int libstm_unix_stream_connect(const char *path, libstm_error_t *err);
#endif
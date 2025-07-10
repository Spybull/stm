#ifndef COMPRESS_DB_H
#define COMPRESS_DB_H
#include "cmd.h"
#include <stdlib.h>

enum {
    STM_COMP_ZSTD,
    STM_COMP_LZ4
};

struct smt_compress_s {
    int type;
    const char *input_data;
    size_t input_size, output_size;
    void **output_data;    
};
typedef struct smt_compress_s stm_compress_t;

typedef int (*fn_cps)(const char *input_data, size_t input_size,
                      size_t *output_size, void **output_data,
                      libstm_error_t *err);

int libstm_zstd_compress(const char *input, size_t input_size, void **out, size_t *output_size, libstm_error_t *err);
int libstm_lz4_compress(const char *input, size_t input_size, void **out, size_t *output_size, libstm_error_t *err);
int libstm_compress(stm_compress_t *opts, fn_cps comp_fn, libstm_error_t *err);

#endif
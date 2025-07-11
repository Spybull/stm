#ifndef COMPRESS_DB_H
#define COMPRESS_DB_H
#include "cmd.h"
#include <stdlib.h>

typedef int (*libstm_comp_fn)(const char *input_data, size_t input_size,
                       size_t *output_size, void **output_data,
                       libstm_error_t *err);

typedef int (*libstm_decomp_fn)(const char *input_data, size_t input_size,
                         size_t *output_size, void **output_data,
                         libstm_error_t *err);

typedef struct {
    int code;
    const char *message;
    const char *source;
} libstm_codec_error_t;

typedef struct {
    int type;
    const char *input_data;
    size_t input_size, output_size;
    void **output_data;    
} libstm_codec_params;

typedef struct {
    const char *name;
    libstm_comp_fn compress;
    libstm_decomp_fn decompress;
    libstm_codec_params_t params;
}libstm_codec;


int libstm_zstd_compress(const char *input, size_t input_size, void **out, size_t *output_size, libstm_error_t *err);
int libstm_lz4_compress(const char *input, size_t input_size, void **out, size_t *output_size, libstm_error_t *err);
int libstm_compress(stm_compress_t *opts, fn_cps comp_fn, libstm_error_t *err);

#endif
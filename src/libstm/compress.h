#ifndef COMPRESS_H
#define COMPRESS_H
#include <stdlib.h>
#include "error.h"

enum { LIBSTM_COMP_ZSTD };

typedef struct {
    int compress_lvl;    
} libstm_compress_opts;

typedef struct {
    int type;
    const char *input_data;
    size_t input_size, output_size;
    void **output_data;    
} libstm_compress_params;

typedef int (*libstm_comp_fn)(
    libstm_compress_opts *opts,
    const char *input_data,
    size_t input_size,
    void **output_data,
    size_t *output_size,
    libstm_error_t *err);

typedef int (*libstm_decomp_fn)(
    libstm_compress_opts *opts,
    const char *input_data,
    size_t input_size,
    void **output_data,
    size_t *output_size,
    libstm_error_t *err);


STM_API int libstm_compress(int type, const char *input, size_t input_size, size_t *output_size, void **output_data, libstm_compress_opts *opts, libstm_error_t *err);
STM_API int libstm_decompress(const char *input, size_t input_size, size_t *output_size, void **output_data, libstm_error_t *err);
#endif
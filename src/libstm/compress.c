#include "compress.h"
#include "utils.h"

#include <zstd.h>
#include <lz4.h>

int
libstm_zstd_compress(const char *input, size_t input_size, void **out, size_t *output_size, libstm_error_t *err)
{
    size_t bound = ZSTD_compressBound(input_size);
    *out = xmalloc(bound);

    size_t compressed = ZSTD_compress(*out, bound, input, input_size, ZSTD_maxCLevel()); /* parametrize? */
    if (ZSTD_isError(compressed)) {
        free(*out);
        return stm_make_error(err, 0, "failed to compress: %s", ZSTD_getErrorName(compressed));
    }

    *output_size = compressed;
    return 0;
}

int
libstm_lz4_compress(const char *input, size_t input_size, void **out, size_t *output_size, libstm_error_t *err) {
    int bound = LZ4_compressBound(input_size);
    *out = xmalloc(bound);
    int compressed = LZ4_compress_default(input, *out, input_size, bound);
    if (compressed <= 0)
        return stm_make_error(err, 0, "failed to compress lz4");

    *output_size = compressed;
    return 0;
}

int
libstm_compress(stm_compress_t *opts, fn_cps comp_fn, libstm_error_t *err)
{
    int rc = 0;
    rc = comp_fn(opts->input_data, opts->input_size,
                &opts->output_size, &opts->output_data, err);
    return rc < 0 ? STM_COMPRESSION_ERROR : 0;
}
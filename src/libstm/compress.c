#include "compress.h"
#include "utils.h"

#include <zstd.h>
#include <stdint.h>

static int
libstm_zstd_compress(libstm_compress_opts *opts, const char *input, size_t input_size,
                    size_t *output_size, void **output_data, libstm_error_t *err)
{
    int lvl = opts ? opts->compress_lvl : 3;
    size_t bound = ZSTD_compressBound(input_size);
    *output_data = xmalloc(bound);

    size_t compressed = ZSTD_compress(*output_data, bound, input, input_size, lvl);
    if (ZSTD_isError(compressed)) {
        free(*output_data);
        return stm_make_error(err, 0, "failed to compress: %s", ZSTD_getErrorName(compressed));
    }

    *output_size = compressed;
    return 0;
}

stm_unused static int
libstm_zstd_decompress(libstm_compress_opts *opts stm_unused, const char *input, size_t input_size,
                      size_t *output_size, void **output_data, libstm_error_t *err)
{
    unsigned long long rsize = ZSTD_getFrameContentSize(input, input_size);
    if (rsize == ZSTD_CONTENTSIZE_ERROR || rsize == ZSTD_CONTENTSIZE_UNKNOWN)
        return stm_make_error(err, 0, "failed to get frame content size");

    *output_data = xmalloc((size_t)rsize);
    size_t result = ZSTD_decompress(*output_data, (size_t)rsize, input, input_size);
    if (ZSTD_isError(result)) {
        free(*output_data);
        return stm_make_error(err, 0, "failed to decompress: %s", ZSTD_getErrorName(result));
    }

    *output_size = result;
    return 0;
}


int
libstm_compress(int type, const char *input, size_t input_size, size_t *output_size, void **output_data, libstm_compress_opts *opts, libstm_error_t *err)
{
    int rc = 0;
    if (type == LIBSTM_COMP_ZSTD)
        rc = libstm_zstd_compress(opts, input, input_size, output_size, output_data, err);
    return rc;
}

int
libstm_decompress(const char *input, size_t input_size, size_t *output_size, void **output_data, libstm_error_t *err)
{
    int rc = 0;
    uint32_t magic;
    memcpy(&magic, input, sizeof(magic));

    if (magic == ZSTD_MAGICNUMBER)
        rc = libstm_zstd_decompress(NULL, input, input_size, output_size, output_data, err);

    return rc;
}


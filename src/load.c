#include "load.h"
#include <argp.h>
#include <csv.h>

#include "libstm/db.h"
#include "libstm/utils.h"
#include "libstm/sec.h"
#include "libstm/formatter.h"

static const char *file_path = NULL;

static struct argp_option options[] = {
    { "file", 'f', "FILE", 0, "load data from file", 0},
    { 0, }
};

static error_t
parse_opt(int key, char *arg stm_unused, struct argp_state *state stm_unused) {

    switch (key)
    {
        case 'f':
            file_path = arg;
        break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char doc[] = "STM load";
static char args_doc[] = "load";
static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

#define MAX_FIELDS 16
#define MAX_FIELD_LEN 256
struct Row {
    char fields[MAX_FIELDS][MAX_FIELD_LEN];
    int field_count;
};

static void field_cb(void *s, size_t len, void *data) {
    struct Row *row = data;
    if (row->field_count < MAX_FIELDS) {
        strncpy(row->fields[row->field_count], (char *)s, len);
        row->fields[row->field_count][len] = '\0';
        row->field_count++;
    }
}

static void row_cb(int c stm_unused, void *data) {
    struct Row *row = data;

    printf("Parsed row with %d fields:\n", row->field_count);
    for (int i = 0; i < row->field_count; i++) {
        printf("  [%d]: %s\n", i, row->fields[i]);
    }

    row->field_count = 0;
}

int
stm_command_load(stm_glob_args *glob_args stm_unused, int argc, char **argv, libstm_error_t *err stm_unused)
{
    int rc = 0;
    cleanup_file FILE *fp = NULL;

    argp_parse(&argp, argc, argv, 0, 0, 0);

    char cur_dir[PATH_MAX] = {0};
    snprintf(cur_dir, PATH_MAX, "%s/%s", glob_args->call_path, file_path); // TODO: check path type

    fp = fopen(cur_dir, "r");
    if (fp == NULL)
        return stm_make_error(err, errno, "failed to load data from file `%s` ", cur_dir);
    
    struct csv_parser parser;
    char delim = ',';
    char buf[1024];
    struct Row row = { .field_count = 0 };
    size_t bytes_read;

    if (csv_init(&parser, CSV_STRICT) != 0) {
        fprintf(stderr, "Failed to init parser\n");
        return 1;
    }

    csv_set_delim(&parser, delim);
    while ((bytes_read = fread(buf, 1, sizeof(buf), fp)) > 0) {
        if (csv_parse(&parser, buf, bytes_read, field_cb, row_cb, &row) != bytes_read) {
            fprintf(stderr, "Parse error: %s\n", csv_strerror(csv_error(&parser)));
            return 1;
        }
    }

    csv_fini(&parser, field_cb, row_cb, &row);
    csv_free(&parser);

    printf("Load data from %s\n", file_path);
    // glob_args->pdb = libstm_db_auth(NULL, NULL, glob_args->stmd_creds_pid_path, glob_args->stmd_creds_sock_path, err);
    // rc = libstm_fmt_dump_as_csv(glob_args->pdb, err);

    return rc;
}
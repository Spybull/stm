#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define FNV_OFFSET 2166136261
#define FNV_PRIME 16777619

unsigned int
hash_string (const char *s)
{
	unsigned int i;

	for (i = FNV_OFFSET; *s; s++) {
		i += (i<<1) + (i<<4) + (i<<7) + (i<<8) + (i<<24);
		i ^= *s;
	}

	return i;
}

int
main(int argc, char *args[])
{
    if (argc == 1) {
        printf("Usage: %s string0 [string1...stringN]\n", args[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; ++i) {
        printf("%s = 0x%x\n", args[i], hash_string(args[i]));
    }

    return 0;
}
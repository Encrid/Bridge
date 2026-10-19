/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "file_bytes.h"

static long measure_file(FILE *f)
{
    long size = 0;

    if (!fseek(f, 0, SEEK_END))
        size = ftell(f);
    if (size <= 0 || fseek(f, 0, SEEK_SET))
        return (0);
    return (size);
}

char *read_whole(const char *path, size_t *len)
{
    FILE *f = fopen(path, "rb");
    char *data = nullptr;
    long size;

    if (!f)
        return (nullptr);
    size = measure_file(f);
    if (size)
        data = calloc((size_t)size + 1, 1);
    if (data && fread(data, 1, (size_t)size, f) != (size_t)size) {
        free(data);
        data = nullptr;
    }
    fclose(f);
    *len = (size_t)size;
    return (data);
}

uint64_t hash_bytes(const char *bytes, size_t len)
{
    uint64_t mark = 14695981039346656037ull;

    while (len--)
        mark = (mark ^ (unsigned char)*bytes++) * 1099511628211ull;
    return (mark);
}

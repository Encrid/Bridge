/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "windows_x86_64.h"

[[clang::no_builtin]]
int memcmp(const void *left, const void *right, size_t len)
{
    const unsigned char *a = left;
    const unsigned char *b = right;

    for (size_t i = 0; i < len; i++)
        if (a[i] != b[i])
            return (a[i] - b[i]);
    return (0);
}

[[clang::no_builtin]]
size_t strlen(const char *text)
{
    size_t len = 0;

    while (text[len])
        len++;
    return (len);
}

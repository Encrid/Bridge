/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "schema_reader.h"

bool schema_is_name(const schema_reader_t *r)
{
    char c = r->word[0];

    if (r->cut)
        return (false);
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_');
}

bool schema_get_integer(const schema_reader_t *r, int64_t *value, bool *below)
{
    char first = r->word[0];
    char *end;

    if (r->cut || (!(first >= '0' && first <= '9') && first != '-'))
        return (false);
    *below = first == '-';
    errno = 0;
    if (*below)
        *value = strtoll(r->word, &end, 0);
    else
        *value = (int64_t)strtoull(r->word, &end, 0);
    return (!*end && !errno);
}

int schema_find_given(const schema_reader_t *r)
{
    uint32_t i = 0;

    while (i < SCHEMA_GIVEN_COUNT && !schema_is(r, SCHEMA_GIVEN[i].name))
        i++;
    if (i == SCHEMA_GIVEN_COUNT)
        return (-1);
    return ((int)i);
}

char *schema_copy_text(const char *text)
{
    size_t n = strlen(text) + 1;
    char *copy = malloc(n);

    if (copy)
        memcpy(copy, text, n);
    return (copy);
}

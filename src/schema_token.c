/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/schema_token.c -- what a token of a descriptor is: a name, an integer,
** a type that is given.
*/

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "schema_reader.h"

/*
** schema_is_name
**
** Answers whether the token is a name a field or a constant may have.
*/
bool schema_is_name(const schema_reader_t *r)
{
    char c = r->word[0];

    if (r->cut)
        return (false);
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_');
}

/*
** schema_get_integer
**
** Reads the token as an integer. One below zero is read signed, and any
** other unsigned, so that the bits of a bitmask of 64 are kept.
*/
bool schema_get_integer(const schema_reader_t *r, int64_t *value, bool *below)
{
    char first = r->word[0];
    char *end;

    if (!(first >= '0' && first <= '9') && first != '-')
        return (false);
    *below = first == '-';
    errno = 0;
    if (*below)
        *value = strtoll(r->word, &end, 0);
    else
        *value = (int64_t)strtoull(r->word, &end, 0);
    return (!*end && end != r->word && !errno);
}

/*
** schema_find_given
**
** Answers which of SCHEMA_GIVEN the token names, or -1.
*/
int schema_find_given(const schema_reader_t *r)
{
    uint32_t i = 0;

    while (i < SCHEMA_GIVEN_COUNT && !schema_is(r, SCHEMA_GIVEN[i].name))
        i++;
    if (i == SCHEMA_GIVEN_COUNT)
        return (-1);
    return ((int)i);
}

/*
** schema_copy_text
**
** Answers a copy of a text that free gives back, or nullptr.
*/
char *schema_copy_text(const char *text)
{
    size_t n = strlen(text) + 1;
    char *copy = malloc(n);

    if (copy)
        memcpy(copy, text, n);
    return (copy);
}

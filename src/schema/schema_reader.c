/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "schema_reader.h"

static bool is_name_char(char c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return (true);
    return ((c >= '0' && c <= '9') || c == '_');
}

static size_t read_run(schema_reader_t *r)
{
    size_t n = 0;
    bool more = true;

    while (more) {
        if (n + 1 < sizeof(r->word)) {
            r->word[n] = *r->at;
            n++;
        } else {
            r->cut = true;
        }
        r->at++;
        more = is_name_char(*r->at);
    }
    return (n);
}

void schema_read_next(schema_reader_t *r)
{
    size_t n = 0;
    const char *at = r->at;

    r->cut = false;
    while (*at == ' ' || *at == '\t' || *at == '\n' || *at == '\r')
        at++;
    r->at = at;
    if (is_name_char(*at) || (*at == '-' && is_name_char(at[1]))) {
        n = read_run(r);
    } else if (*at) {
        r->word[0] = *at;
        r->at++;
        n = 1;
    }
    r->word[n] = 0;
}

int schema_refuse(schema_reader_t *r, const char *what)
{
    const char *word = r->word[0] ? r->word : "the end of it";

    snprintf(r->source->why, r->source->why_len, what, word);
    return (1);
}

bool schema_is(const schema_reader_t *r, const char *word)
{
    return (!strcmp(r->word, word));
}

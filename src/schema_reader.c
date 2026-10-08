/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/schema_reader.c -- a descriptor read a token at a time.
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

/*
** read_run
**
** Reads a run of name characters into `word`, whose first is already known
** to begin one. Answers how many were kept: a run too long is cut.
*/
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

/*
** schema_read_next
**
** Reads the next token into `word`. A minus sign begins a run when a name
** character follows it, which is how a number below zero is one token.
*/
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

/*
** schema_refuse
**
** Writes why the descriptor is refused, with the token it stopped at, or
** "the end of it" where the text ended. Answers 1.
*/
int schema_refuse(schema_reader_t *r, const char *what)
{
    const char *word = r->word[0] ? r->word : "the end of it";

    snprintf(r->source->why, r->source->why_len, what, word);
    return (1);
}

/*
** schema_is
**
** Answers whether the token is `word`.
*/
bool schema_is(const schema_reader_t *r, const char *word)
{
    return (!strcmp(r->word, word));
}

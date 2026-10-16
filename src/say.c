/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "say.h"

static void send_record(bridge_t *b, const say_t *about, const char *text)
{
    bridge_record_t r = {};

    r.identity = about->m ? about->m->identity : nullptr;
    r.system = about->s ? about->s->name : nullptr;
    r.text = text;
    r.len = (uint32_t)strlen(text);
    r.barriers = b->barriers;
    r.level = about->level;
    b->sink(b->ud, &r);
}

void say_error(bridge_t *b, const char *format, ...)
{
    say_t about = { nullptr, nullptr, BRIDGE_ERR };
    char text[512];
    va_list args;

    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    send_record(b, &about, text);
}

void say_about(bridge_t *b, const say_t *about, const char *format, ...)
{
    char text[512];
    va_list args;

    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    send_record(b, about, text);
}

char *copy_text(const char *s, size_t n)
{
    char *copy = malloc(n + 1);

    if (!copy)
        return (nullptr);
    memcpy(copy, s, n);
    copy[n] = 0;
    return (copy);
}

int bridge_lacks(bridge_t *b, const void *got, const char *what)
{
    if (got)
        return (0);
    say_error(b, "out of memory: there was none for %s", what);
    return (1);
}

/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/say.c -- a record of Bridge-ASM700's own, handed to the host's sink.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "say.h"

/*
** send_record
**
** Hands the sink one record: the text, whose it is about, and the barrier it
** was said at.
*/
static void send_record(bridge_t *b, const say_t *about, const char *text)
{
    bridge_record_t r = {};

    r.identity = about->m ? about->m->identity : nullptr;
    r.system = about->s ? about->s->name : nullptr;
    r.text = text;
    r.len = (uint32_t)strlen(text);
    r.when = b->barriers << 16;
    r.level = about->level;
    b->sink(b->ud, &r);
}

/*
** say_error
**
** Hands the sink an error of Bridge-ASM700's own, about no module.
*/
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

/*
** say_about
**
** Hands the sink a record about a module or one of its systems.
*/
void say_about(bridge_t *b, const say_t *about, const char *format, ...)
{
    char text[512];
    va_list args;

    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    send_record(b, about, text);
}

/*
** copy_text
**
** Answers a copy of the first `n` bytes of a text, with a zero after them.
*/
char *copy_text(const char *s, size_t n)
{
    char *copy = malloc(n + 1);

    if (!copy)
        return (nullptr);
    memcpy(copy, s, n);
    copy[n] = 0;
    return (copy);
}

/*
** bridge_lacks
**
** Answers 1, with a record, if `got` is nullptr.
*/
int bridge_lacks(bridge_t *b, const void *got, const char *what)
{
    if (got)
        return (0);
    say_error(b, "out of memory: there was none for %s", what);
    return (1);
}

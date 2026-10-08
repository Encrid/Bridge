/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/support/native_refusing_backend.c -- the native backend's first
** entries, as a backend that refuses would answer them: one that says each
** setting it was handed and does not start, for the case that reads what
** crosses the backend seam. With native_refusing_memory.c it stands in the
** place of backends/native/native_backend.c.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bridge_backend.h"

/*
** say_setting
**
** Writes a setting after what `err` holds: its key, and its value as its
** kind says to read it.
*/
static void say_setting(backend_err_t *err, const backend_setting_t *s)
{
    size_t at = strlen(err->text);
    char *to = err->text + at;
    size_t room = sizeof(err->text) - at;

    if (s->kind == BACKEND_BOOL)
        snprintf(to, room, " %s=%s", s->key, s->v.b ? "true" : "false");
    if (s->kind == BACKEND_INT)
        snprintf(to, room, " %s=%lld", s->key, (long long)s->v.i);
    if (s->kind == BACKEND_FLOAT)
        snprintf(to, room, " %s=%.1f", s->key, s->v.f);
    if (s->kind == BACKEND_STRING)
        snprintf(to, room, " %s='%s'", s->key, s->v.s);
}

/*
** backend_create
**
** Makes the backend if it is handed no setting: a byte, since it keeps
** nothing. Handed some, it says each and does not start.
*/
BACKEND_EXPORT backend_t *backend_create(const backend_setting_t *settings,
    uint32_t count, backend_err_t *err)
{
    uint32_t i = 0;

    if (!count)
        return (calloc(1, 1));
    snprintf(err->text, sizeof(err->text), "it was handed");
    while (i < count) {
        say_setting(err, &settings[i]);
        i++;
    }
    return (nullptr);
}

/*
** backend_destroy
**
** Frees the backend.
*/
BACKEND_EXPORT void backend_destroy(backend_t *b)
{
    free(b);
}

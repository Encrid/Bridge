/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "wasm_inside.h"

static const char WASM_MARKER[] = "wasm trap: ";

void wasm_drop(wasmtime_error_t *error)
{
    if (error)
        wasmtime_error_delete(error);
}

static const char *find_reason(const wasm_byte_vec_t *msg, int *n)
{
    constexpr size_t marker = sizeof(WASM_MARKER) - 1;
    const char *text = msg->data;
    const char *end = msg->data + msg->size;
    const char *p = text;

    while (p && p + marker <= end) {
        if (!memcmp(p, WASM_MARKER, marker))
            text = p + marker;
        p++;
    }
    *n = 0;
    if (!text)
        return ("");
    p = text;
    while (p < end && *p != '\n')
        p++;
    *n = p - text > 200 ? 200 : (int)(p - text);
    return (text);
}

int wasm_fail(backend_err_t *err, const char *what, wasmtime_error_t *error,
    wasm_trap_t *trap)
{
    wasm_byte_vec_t msg = {};
    const char *text;
    int n;

    if (error)
        wasmtime_error_message(error, &msg);
    if (!error && trap)
        wasm_trap_message(trap, &msg);
    text = find_reason(&msg, &n);
    if (err)
        snprintf(err->text, sizeof(err->text), "%s%s%.*s", what,
            n ? ": " : "", n, text);
    if (msg.data)
        wasm_byte_vec_delete(&msg);
    wasm_drop(error);
    if (trap)
        wasm_trap_delete(trap);
    return (-1);
}

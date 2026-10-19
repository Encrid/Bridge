/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "wasm_inside.h"

static const char WASM_REFUSED[] = "refused by Bridge";

wasm_offered_t *wasm_make_offered(const backend_entry_t *entry)
{
    const backend_sig_t *sig = entry->sig;
    wasm_offered_t *h = calloc(1, sizeof(*h));

    h->fn = entry->fn;
    h->ud = entry->ud;
    h->params = sig->param_count;
    h->results = sig->result_count;
    memcpy(h->param_kind, sig->params, sig->param_count);
    memcpy(h->result_kind, sig->results, sig->result_count);
    return (h);
}

static void read_args(const wasm_offered_t *h, const wasmtime_val_raw_t *raw,
    uint64_t *in)
{
    uint32_t k = 0;

    while (k < h->params) {
        if (h->param_kind[k] == BACKEND_I32)
            in[k] = (uint32_t)raw[k].i32;
        else
            in[k] = (uint64_t)raw[k].i64;
        k++;
    }
}

static void write_results(const wasm_offered_t *h, const uint64_t *out,
    wasmtime_val_raw_t *raw)
{
    uint32_t k = 0;

    while (k < h->results) {
        if (h->result_kind[k] == BACKEND_I32)
            raw[k].i32 = (int32_t)out[k];
        else
            raw[k].i64 = (int64_t)out[k];
        k++;
    }
}

wasm_trap_t *wasm_call_offered(void *env,
    [[maybe_unused]] wasmtime_caller_t *caller, wasmtime_val_raw_t *raw,
    [[maybe_unused]] size_t count)
{
    const wasm_offered_t *h = env;
    uint64_t in[16] = {};
    uint64_t out[4] = {};

    read_args(h, raw, in);
    if (h->fn(h->ud, in, out))
        return (wasmtime_trap_new(WASM_REFUSED, sizeof(WASM_REFUSED) - 1));
    write_results(h, out, raw);
    return (nullptr);
}

void wasm_make_types(wasm_valtype_vec_t *vec,
    const enum backend_type *kinds, uint32_t count)
{
    uint32_t k = 0;

    wasm_valtype_vec_new_uninitialized(vec, count);
    while (k < count) {
        if (kinds[k] == BACKEND_I32)
            vec->data[k] = wasm_valtype_new(WASM_I32);
        else
            vec->data[k] = wasm_valtype_new(WASM_I64);
        k++;
    }
}

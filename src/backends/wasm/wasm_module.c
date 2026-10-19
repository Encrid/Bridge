/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "wasm_inside.h"

static wasmtime_module_t *compile(wasm_engine_t *engine,
    const backend_source_t *source, backend_err_t *err)
{
    const char *what = "this backend compiles a module from its bytes, and "
        "was handed none";
    wasmtime_module_t *module = nullptr;
    wasmtime_error_t *error = nullptr;

    if (source->bytes && source->len) {
        what = "not a module this backend can compile";
        error = wasmtime_module_new(engine, source->bytes, source->len,
            &module);
    }
    if (!module)
        wasm_fail(err, what, error, nullptr);
    return (module);
}

BACKEND_EXPORT backend_module_t *module_compile(backend_t *b,
    const backend_source_t *source, backend_err_t *err)
{
    enum wasm_engine kind = 0;
    wasmtime_module_t *module;
    backend_module_t *m;

    if (source->demands & BACKEND_WORK_LIMIT)
        kind |= WASM_METERED;
    if (source->demands & BACKEND_TIMEOUT)
        kind |= WASM_INTERRUPTIBLE;
    module = compile(wasm_get_engine(b, kind), source, err);
    if (!module)
        return (nullptr);
    m = calloc(1, sizeof(*m));
    m->b = b;
    m->kind = kind;
    m->module = module;
    wasmtime_module_imports(module, &m->imports);
    return (m);
}

static enum backend_import_kind get_kind(const wasm_importtype_t *it)
{
    wasm_externkind_t kind = wasm_externtype_kind(wasm_importtype_type(it));

    if (kind == WASM_EXTERN_FUNC)
        return (BACKEND_IMPORT_FUNC);
    if (kind == WASM_EXTERN_MEMORY)
        return (BACKEND_IMPORT_MEMORY);
    return (BACKEND_IMPORT_OTHER);
}

BACKEND_EXPORT uint32_t module_list_imports(backend_module_t *m,
    backend_import_t *out, uint32_t max)
{
    const wasm_importtype_t *it;
    const wasm_name_t *ns;
    const wasm_name_t *field;
    uint32_t n = (uint32_t)m->imports.size;
    uint32_t i = 0;

    while (i < n && i < max) {
        it = m->imports.data[i];
        ns = wasm_importtype_module(it);
        field = wasm_importtype_name(it);
        out[i].ns = ns->data;
        out[i].ns_len = ns->size;
        out[i].field = field->data;
        out[i].field_len = field->size;
        out[i].kind = get_kind(it);
        i++;
    }
    return (n);
}

BACKEND_EXPORT void module_destroy(backend_module_t *m)
{
    wasm_importtype_vec_delete(&m->imports);
    wasmtime_module_delete(m->module);
    free(m);
}

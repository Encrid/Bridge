/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/module_instance.c -- a module's image compiled and instantiated. A
** module is handed the memories it imports, and no other.
**
** backend.md, *Memory is adopted, not created*
*/

#include <string.h>

#include "module.h"

static bool is_text(const char *at, size_t len, const char *text)
{
    return (len == strlen(text) && !memcmp(at, text, len));
}

/*
** lend_memory
**
** Writes the memory an import asks for, if it is one of the two there are:
** the shared block, or the module's private one. Answers how many it wrote.
*/
static uint32_t lend_memory(const module_t *m, const backend_import_t *im,
    backend_memory_import_t *lent)
{
    const char *ns = BRIDGE_NAMESPACE;

    if (im->kind != BACKEND_IMPORT_MEMORY)
        return (0);
    if (!is_text(im->ns, im->ns_len, ns))
        return (0);
    if (is_text(im->field, im->field_len, BRIDGE_SHARED)) {
        *lent = (backend_memory_import_t){ ns, BRIDGE_SHARED, m->on->shared };
        return (1);
    }
    if (!is_text(im->field, im->field_len, BRIDGE_PRIVATE))
        return (0);
    *lent = (backend_memory_import_t){ ns, BRIDGE_PRIVATE, m->memory };
    return (1);
}

static uint32_t lend_memories(const module_t *m,
    backend_memory_import_t *memories)
{
    backend_import_t imports[64];
    uint32_t count = m->on->api.module_list_imports(m->image, imports, 64);
    uint32_t lent = 0;
    uint32_t i = 0;

    while (i < count && i < 64 && lent < 2) {
        lent += lend_memory(m, &imports[i], &memories[lent]);
        i++;
    }
    return (lent);
}

/*
** module_make_instance
**
** Compiles a module's image, reserves its private memory, and instantiates
** it with the memories it imports and the ABI.
*/
int module_make_instance(module_t *m, const module_start_t *st,
    backend_err_t *err)
{
    const bridge_artifact_t *a = st->a;
    open_backend_t *o = m->on;
    backend_source_t source = { a->bytes, a->len, a->path, st->demands, 0 };
    backend_memory_import_t memories[2];
    uint32_t count;

    m->image = o->api.module_compile(o->be, &source, err);
    if (!m->image || module_take_private(m, &a->cfg, err))
        return (1);
    block_write_header(m->b, m);
    count = lend_memories(m, memories);
    m->instance = o->api.instance_create(m->image, memories, count, err);
    if (!m->instance || abi_define(m, err))
        return (1);
    if (a->cfg.memory_limit > 0)
        o->api.instance_set_memory_limit(m->instance,
            (uint64_t)a->cfg.memory_limit);
    return (0);
}

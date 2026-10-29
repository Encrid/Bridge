/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "module.h"

static bool is_text(const char *at, size_t len, const char *text)
{
    return (len == strlen(text) && !memcmp(at, text, len));
}

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

static backend_instance_t *create_instance(const module_t *m,
    backend_err_t *err)
{
    const backend_api_t *api = &m->on->api;
    uint32_t count = api->module_list_imports(m->image, nullptr, 0);
    backend_import_t *imports = calloc((size_t)count + 1, sizeof(*imports));
    backend_memory_import_t memories[2];
    uint32_t lent = 0;
    uint32_t i = 0;

    if (!imports) {
        module_fail(err, "out of memory: there was none for the list of its "
            "imports");
        return (nullptr);
    }
    (void)api->module_list_imports(m->image, imports, count);
    while (i < count && lent < 2) {
        lent += lend_memory(m, &imports[i], &memories[lent]);
        i++;
    }
    free(imports);
    return (api->instance_create(m->image, memories, lent, err));
}

int module_make_instance(module_t *m, const module_start_t *st,
    backend_err_t *err)
{
    const bridge_artifact_t *a = st->a;
    open_backend_t *o = m->on;
    backend_source_t source = { a->bytes, a->len, a->path, st->demands, 0 };

    m->image = o->api.module_compile(o->be, &source, err);
    if (!m->image || module_take_private(m, &a->cfg, err))
        return (1);
    if (block_write_header(m->w, m))
        return (module_fail(err, "the entity index is outside the block"));
    m->instance = create_instance(m, err);
    if (!m->instance || abi_define(m, err))
        return (1);
    if (a->cfg.memory_limit > 0)
        o->api.instance_set_memory_limit(m->instance,
            (uint64_t)a->cfg.memory_limit);
    return (0);
}

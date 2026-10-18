/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "abi_declare.h"

static int parse_descriptor(declaring_t *g)
{
    char *desc = copy_text(g->desc, g->d.desc_len);
    char why[200];
    schema_source_t source = { g->d.kind, desc, abi_find_type, g, why };
    int refused;

    if (!desc)
        return (abi_refuse(g->m, "out of memory: there was none for the "
            "descriptor of '%s'", g->name));
    source.why_len = sizeof(why);
    *g->bare = 0;
    refused = schema_parse(&source, &g->s);
    *g->bare = ':';
    if (!refused) {
        g->text = desc;
        return (0);
    }
    free(desc);
    return (abi_refuse(g->m, "type '%s': its descriptor %s", g->name, why));
}

int abi_check_size(declaring_t *g, uint32_t size, uint32_t align)
{
    const bridge_declaration_t *d = &g->d;

    if (size == d->size && align == d->align)
        return (0);
    return (abi_refuse(g->m, "type '%s' is %u bytes aligned to %u in the "
        "module and %u aligned to %u by its descriptor", g->name, d->size,
        d->align, size, align));
}

bool abi_is_same_layout(const declared_type_t *c, const declaring_t *g)
{
    if (c->kind != g->d.kind || c->desc_len != g->d.desc_len)
        return (false);
    return (!c->desc_len || !memcmp(c->desc, g->desc, c->desc_len));
}

static int check_room(declaring_t *g)
{
    module_t *m = g->m;
    const bridge_t *b = m->b;
    declared_type_t *was = g->was;
    enum bridge_storage kept = g->d.storage;
    bool changed = was && !abi_is_same_layout(was, g);

    changed = changed || (was && abi_names_superseded(was));
    if (changed && abi_relay(m, was, g->d.kind, &g->s))
        return (1);
    if (b->type_count >= b->max_types)
        return (abi_refuse(m, "type '%s' is one more than [budget] types of "
            "%u", g->name, b->max_types));
    if (kept == BRIDGE_SPARSE_SET && !storage_count_free_sets(b->storage))
        return (abi_refuse(m, "type '%s' is one more than [budget] "
            "sparse_sets of %u", g->name, b->sparse_sets));
    if (kept != BRIDGE_DIRECT_INDEXED || storage_count_free_direct(b->storage))
        return (0);
    return (abi_refuse(m, "type '%s' is one more than [budget] "
        "direct_indexed of %u", g->name, b->direct_indexed));
}

int abi_read_layout(declaring_t *g)
{
    memset(&g->s, 0, sizeof(g->s));
    g->s.align = g->d.align;
    if (g->d.kind != BRIDGE_TAG && parse_descriptor(g))
        return (1);
    if (!abi_check_size(g, g->s.size, g->s.align) && !check_room(g))
        return (0);
    schema_free(&g->s);
    free(g->text);
    g->text = nullptr;
    return (1);
}

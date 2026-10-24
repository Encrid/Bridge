/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "abi_declare.h"

static const uint8_t ELEMENT_SIZES[SCHEMA_STRUCT + 1] = {
    [SCHEMA_BOOL] = 1,
    [SCHEMA_CHAR] = 1,
    [SCHEMA_I8] = 1,
    [SCHEMA_U8] = 1,
    [SCHEMA_I16] = 2,
    [SCHEMA_U16] = 2,
    [SCHEMA_I32] = 4,
    [SCHEMA_U32] = 4,
    [SCHEMA_F32] = 4,
    [SCHEMA_I64] = 8,
    [SCHEMA_U64] = 8,
    [SCHEMA_F64] = 8,
    [SCHEMA_TEXT] = 8
};

bool abi_accepts(module_t *m, const char *name, uint32_t rows)
{
    uint32_t i = 0;

    while (i < m->loss_count && strcmp(m->losses[i], name))
        i++;
    if (i == m->loss_count)
        return (false);
    m->loss_rows[i] = rows + 1;
    return (true);
}

uint32_t abi_get_element_size(const schema_field_t *f)
{
    const declared_type_t *type = f->type;

    if (f->kind <= SCHEMA_STRUCT && ELEMENT_SIZES[f->kind])
        return (ELEMENT_SIZES[f->kind]);
    return (type->size);
}

void abi_add_run(relaying_t *l, uint32_t from, uint32_t to, uint32_t len)
{
    size_t room = (l->run_count + 1) * sizeof(*l->runs);
    storage_run_t *runs = realloc(l->runs, room);

    if (!runs) {
        l->short_of = true;
        return;
    }
    l->runs = runs;
    l->runs[l->run_count] = (storage_run_t){ from, to, len };
    l->run_count++;
}

void abi_say_lost(relaying_t *l, const char *name)
{
    size_t room = sizeof(l->lost);

    if (!l->rows || abi_accepts(l->m, name, l->rows) || l->lost_len >= room)
        return;
    l->lost_len += (size_t)snprintf(l->lost + l->lost_len,
        room - l->lost_len, "%s'%s'", l->lost_len ? ", " : "", name);
}

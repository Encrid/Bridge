/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "storage_test.h"

bridge_system_t *storage_test_make_system(module_t *m, uint32_t terms,
    uint32_t changes, uint32_t words)
{
    bridge_system_t *s = calloc(1, sizeof(*s) + terms * sizeof(system_term_t)
        + changes * sizeof(declared_type_t *) + words * sizeof(uint64_t)
        + (terms + changes) * sizeof(system_use_t));

    if (!s)
        return (nullptr);
    s->m = m;
    s->terms = terms;
    s->changes = (declared_type_t **)(void *)&s->term[terms];
    s->stated_before = (uint64_t *)(void *)&s->changes[changes];
    s->use = (system_use_t *)(void *)&s->stated_before[words];
    return (s);
}

int storage_test_build_all(bridge_world_t *w, char *why, size_t why_len)
{
    schedule_mark_all(w);
    return (schedule_build(w, why, why_len));
}

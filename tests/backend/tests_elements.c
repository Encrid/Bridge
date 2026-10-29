/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "tests_inside.h"

BACKEND_EXPORT void instance_set_element_limit(backend_instance_t *i,
    uint64_t elements)
{
    i->element_limit = elements;
}

int tests_grow_elements(const test_api_t *api, uint64_t elements)
{
    backend_instance_t *i = tests_get_instance(api);

    if (i->element_limit && elements > i->element_limit - i->elements)
        return (0);
    i->elements += elements;
    return (1);
}

/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "nsparse.h"
#include "native_measured.h"

uint32_t nsparse_a;
uint32_t nsparse_s;
uint64_t nsparse_ids[COUNT];

BRIDGE_NATIVE(spawn)
{
    a_t value = {};

    while (value.v < COUNT) {
        nsparse_ids[value.v] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, nsparse_ids[value.v], nsparse_a, value);
        value.v++;
    }
    return (0);
}

NATIVE_MEASURED BRIDGE_NATIVE(attach_b)
{
    s_t value;
    uint32_t i = 0;

    while (i < COUNT) {
        value.v = 100 + i;
        BRIDGE_NATIVE_ATTACH(api, nsparse_ids[i], nsparse_s, value);
        i += STEP;
    }
    return (0);
}

NATIVE_MEASURED BRIDGE_NATIVE(detach_b)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->detach(api, nsparse_ids[i], nsparse_s);
        i += STEP;
    }
    return (0);
}

BRIDGE_NATIVE(destroy)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->destroy(api, nsparse_ids[i]);
        i++;
    }
    if (nsparse_lone)
        api->destroy(api, nsparse_lone);
    return (0);
}

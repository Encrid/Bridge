/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "nsingle.h"

BRIDGE_NATIVE(attach_a)
{
    g_t value = { 10, 1 };
    [[maybe_unused]] a_t other = { 1 };

#if defined(BAD) && BAD == 2
    BRIDGE_NATIVE_ATTACH(api, api->spawn(api), nsingle_g, value);
#elif defined(BAD) && BAD == 3
    BRIDGE_NATIVE_ATTACH(api, 0, nsingle_a, other);
#endif
    BRIDGE_NATIVE_ATTACH(api, 0, nsingle_g, value);
    return (0);
}

BRIDGE_NATIVE(value_a)
{
    g_t value = { 20, 2 };

    BRIDGE_NATIVE_ATTACH(api, 0, nsingle_g, value);
    return (0);
}

BRIDGE_NATIVE(work)
{
    g_t *value = bridge_native_get_single(api, nsingle_g);

    if (value)
        value->wind = 77;
    return (0);
}

BRIDGE_NATIVE(detach_b)
{
    api->detach(api, 0, nsingle_g);
    return (0);
}

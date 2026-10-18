/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "nsparse.h"

uint64_t nsparse_lone;

BRIDGE_NATIVE(detach_b_half)
{
    api->detach(api, nsparse_ids[0], nsparse_s);
    return (0);
}

BRIDGE_NATIVE(value_b)
{
    s_t value = { 900 };

    BRIDGE_NATIVE_ATTACH(api, nsparse_ids[2 % COUNT], nsparse_s, value);
    return (0);
}

BRIDGE_NATIVE(attach_b_half)
{
    s_t value = { 7 };

    nsparse_lone = api->spawn(api);
    BRIDGE_NATIVE_ATTACH(api, nsparse_lone, nsparse_s, value);
    return (0);
}

BRIDGE_NATIVE(destroy_half)
{
    api->destroy(api, nsparse_ids[2 % COUNT]);
    api->destroy(api, nsparse_ids[1 % COUNT]);
    return (0);
}

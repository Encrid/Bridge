/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/packaged/npack.c -- a native module, built from that
** backend's SDK package and nothing else of Bridge, as one outside the
** tree is.
**
** host_api.md, *What a host is handed*
*/

#include "bridge_native.h"
#include "npack.h"

static uint32_t count;
static uint32_t counting;

/*
** advance
**
** Adds one to the count, and says what it has become.
*/
BRIDGE_NATIVE(advance)
{
    const bridge_query_t *q = bridge_native_get_query(api, counting);
    const uint32_t *span = bridge_native_get_span(api, q, 0, 1);
    count_t *c = (count_t *)(api->shared_base + span[2]);
    bridge_line_t line = { "count", 5, 0, 0 };

    c->n += 1;
    line.a0 = c->n;
    bridge_native_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

/*
** init
**
** Declares the component and the system, and spawns one entity.
*/
BRIDGE_NATIVE(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    count_t c = {};

    count = BRIDGE_NATIVE_COMPONENT(api, "packed:Count", count_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = count;
    counting = BRIDGE_NATIVE_ADD_SYSTEM(api, "advance", "update", terms, 1,
        nullptr, 0);
    BRIDGE_NATIVE_ATTACH(api, api->spawn(api), count, c);
    return (0);
}

/*
** reload
**
** Nothing to carry over.
*/
BRIDGE_NATIVE(reload)
{
    return (0);
}

/*
** shutdown
**
** Nothing to release.
*/
BRIDGE_NATIVE(shutdown)
{
    return (0);
}

/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/packaged/pack.c -- a wasm module, built from that
** backend's SDK package and nothing else of Bridge, as one outside the
** tree is.
**
** host_api.md, *What a host is handed*
*/

#include "bridge_wasm.h"
#include "pack.h"

static uint32_t sum;
static uint32_t summing;

/*
** advance
**
** Adds ten to the sum, and says what it has become.
*/
BRIDGE_EXPORT(advance) int advance(void)
{
    uint32_t span = bridge_get_span(summing, 0, 1);
    uint32_t at = BRIDGE_ROW(sum_t, bridge_get_base(span, 0), 0,
        bridge_count_rows(span));

    BRIDGE_SET(sum_t, n, at, BRIDGE_GET(sum_t, n, at) + 10);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "sum", BRIDGE_GET(sum_t, n, at), 0);
    return (0);
}

/*
** init
**
** Declares the component and the system, and spawns one entity.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    sum_t s = { 0 };

    sum = BRIDGE_COMPONENT("packed:Sum", sum_t, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = sum;
    summing = BRIDGE_ADD_SYSTEM("advance", "update", terms, 1, nullptr, 0);
    BRIDGE_ATTACH(bridge_spawn(), sum, s);
    return (0);
}

/*
** reload
**
** Nothing to carry over.
*/
BRIDGE_EXPORT(reload) int reload(void)
{
    return (0);
}

/*
** shutdown
**
** Nothing to release.
*/
BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    return (0);
}

/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_native.h"
#include "native_idle.h"

BRIDGE_NATIVE(init)
{
    if (api->private_base) {
        bridge_header_t *h = bridge_native_get_header(api);
        bridge_line_t line = { "cover", 5, 0, 0 };
        char buf[8] = {};
        uint32_t scratch = h->scratch_used;
        uint32_t head = h->ring_head;

        h->scratch_used = 0;
        bridge_native_put(api, buf, h->scratch_len + 1u);
        bridge_native_lend(api, buf, h->scratch_len + 1u);
        h->scratch_used = h->scratch_len + 1u;
        bridge_native_put(api, buf, 1u);
        bridge_native_lend(api, buf, 1u);
        h->scratch_used = scratch;
        bridge_native_log(api, h->log_level + 1u, &line);
        h->ring_head = h->ring_tail + h->ring_len;
        bridge_native_log(api, BRIDGE_LOG_ERR, &line);
        h->ring_head = head;
    }
    (void)api->spawn(api);
    return (0);
}

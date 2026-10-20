/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_inside.h"

void storage_hand_out(storage_t *r, const uint64_t *ids, uint32_t count)
{
    place_t *p;

    for (uint32_t i = 0; i < count; i++) {
        p = &r->places[(uint32_t)ids[i]];
        if (!p->table && p->row == HELD) {
            p->row = 0;
            r->w->index[(uint32_t)ids[i]] = (bridge_entity_t){ 0, 0,
                p->generation };
        }
    }
}

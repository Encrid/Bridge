/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "say.h"

void say_send(bridge_world_t *w, const bridge_record_t *r)
{
    w->sink(w->ud, r);
}

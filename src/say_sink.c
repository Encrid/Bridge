/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "say.h"

void say_send(bridge_t *b, const bridge_record_t *r)
{
    b->in_sink = true;
    b->sink(b->ud, r);
    b->in_sink = false;
}

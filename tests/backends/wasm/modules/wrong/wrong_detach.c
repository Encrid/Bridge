/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_detach_undeclared(void)
{
    bridge_detach(1, WRONG_NO_SLOT);
}

void wrong_attach_tag_value(void)
{
    uint32_t value = 1;

    bridge_attach(1, wrong_flag, bridge_lend(&value, sizeof(value)));
}

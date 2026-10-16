/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"

static volatile int going = 1;

BRIDGE_EXPORT(init) int init(void)
{
    return (0);
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    if (going)
        test_get_table()->wait(test_get_table(), UINT64_MAX);
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    return (0);
}

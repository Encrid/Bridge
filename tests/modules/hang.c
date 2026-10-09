/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/hang.c -- a module whose shutdown never returns: its
** timeout ends it, and the teardown goes on.
*/

#include "test_offsets.h"

static volatile int going = 1;

/*
** init
**
** Does nothing.
*/
BRIDGE_EXPORT(init) int init(void)
{
    return (0);
}

/*
** shutdown
**
** Never returns.
*/
BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    if (going)
        test_get_table()->wait(test_get_table(), UINT64_MAX);
    return (0);
}

/*
** reload
**
** Does nothing.
*/
BRIDGE_EXPORT(reload) int reload(void)
{
    return (0);
}

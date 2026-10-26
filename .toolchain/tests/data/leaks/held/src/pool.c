/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "grab.h"
#include "pool.h"

void *pool_take(void)
{
    return (malloc(16));
}

note_t *pool_refuse(int ok)
{
    return (ok ? note_new() : nullptr);
}

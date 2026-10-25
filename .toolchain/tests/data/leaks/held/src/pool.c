/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "pool.h"

void *pool_take(void)
{
    return (malloc(16));
}

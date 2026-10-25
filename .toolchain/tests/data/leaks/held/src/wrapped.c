/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#if defined(__STDC_VERSION__)
    #include <stdlib.h>

    #include "pool.h"

void *wrapped_take(void)
{
    return (malloc(4));
}
#endif

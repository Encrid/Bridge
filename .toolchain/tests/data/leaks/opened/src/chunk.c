/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

static char *held;

int chunk_make(void)
{
    held = malloc(32);
    return (held != nullptr);
}

void chunk_drop(void)
{
    free(held);
    held = nullptr;
}

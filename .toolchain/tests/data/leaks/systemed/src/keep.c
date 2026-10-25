/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

int main(int count, [[maybe_unused]] char **words)
{
    printf("%s\n", count > 1 ? "freed" : "kept");
    if (count > 1)
        return (0);
    return (malloc(4) == nullptr);
}

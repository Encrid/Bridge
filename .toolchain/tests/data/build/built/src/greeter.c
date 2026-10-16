/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "greet.h"
#include "system.h"

int main(void)
{
    printf("%s, on %s\n", greeting(), system_name());
    return 0;
}

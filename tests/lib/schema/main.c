/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "schema_test.h"

int main(void)
{
    int failed = schema_run_structs();

    failed += schema_run_constants();
    failed += schema_run_refusals();
    printf("%s\n", failed ? "FAIL" : "every descriptor read as it should, "
        "and every other refused by its word");
    printf("alive\n");
    return (failed != 0);
}

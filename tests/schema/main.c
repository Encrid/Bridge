/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/schema/main.c -- schema: every descriptor there is a row for, read.
*/

#include <stdio.h>

#include "schema_test.h"

/*
** main
**
** Reads every descriptor and says whether each read as it should.
*/
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

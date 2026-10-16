/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "branchy.h"

#define CHECKED(c) (c)

int count_unused(int value)
{
    return (value + 1);
}

static void say_value(int value)
{
    switch (value) {
        case 1:
            printf("one\n");
            break;
        case 2:
            printf("two\n");
            break;
        default:
            printf("other\n");
    }
}

int main(int argc, char **argv)
{
    int value = argc > 1 ? atoi(argv[1]) : 0;
    int flag = argc > 2;

    say_value(value);
    if (value > 0 && flag)
        printf("both\n");
    if (CHECKED(value == 3 || flag))
        printf("checked\n");
    printf("sign %d\n", get_sign(value));
    return (0);
}

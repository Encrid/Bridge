/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "branchy.h"

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
    printf("sign %d\n", get_sign(value));
    return (0);
}

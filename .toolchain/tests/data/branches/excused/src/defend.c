/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

int defend(int value)
{
    if (value < 0)
        return (printf("guard\n"));
    return (printf("ok %d\n", value));
}

int main(int argc, char **argv)
{
    (void)argv;
    return (defend(argc) < 0);
}

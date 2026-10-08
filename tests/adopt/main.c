/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/adopt/main.c -- adopt: the checks, in order.
*/

#include <stdio.h>

#include "adopt.h"

/*
** main
**
** Opens the backend and makes each check. Nothing is buffered: a crash must
** not swallow what was said.
*/
int main(void)
{
    adopt_t t = {};
    backend_instance_t *one;

    setvbuf(stdout, nullptr, _IONBF, 0);
    if (adopt_open(&t))
        return (1);
    one = adopt_make_instance(&t);
    if (adopt_check_write(&t, one) || adopt_check_two_stores(&t, one))
        return (1);
    if (adopt_check_stray(&t, one) || adopt_check_host_function(&t))
        return (1);
    if (adopt_check_timed_calls(&t) || adopt_check_refusals(&t, one))
        return (1);
    t.api.instance_destroy(one);
    t.api.module_destroy(t.module);
    t.api.memory_release(t.memory);
    t.api.backend_destroy(t.be);
    os_release(t.block, t.reserved);
    return (0);
}

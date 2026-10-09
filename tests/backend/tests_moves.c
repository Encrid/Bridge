/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_moves.c -- the four moves of the tests: what a
** module says it does, held against what Bridge set on its instance.
** Nothing is measured: work and waiting are counted, so a case ends the
** same way every time.
**
** limits.md, *What bounds a run*
*/

#include "tests_inside.h"

/*
** end_call
**
** Ends the call in progress, with why: it does not return. Outside a call
** there is none to end, and it returns.
*/
static void end_call(backend_instance_t *i, const char *why)
{
    if (!i->out)
        return;
    i->why = why;
    longjmp(*i->out, 1);
}

/*
** tests_trap
**
** Ends the call, as a module that faults is ended where a backend can
** catch it.
*/
void tests_trap(const test_api_t *api)
{
    end_call(tests_get_instance(api), "the module asked for it");
}

/*
** tests_spend
**
** Counts units of work against what is left of the limit, and ends the
** call past it. With no limit set, nothing is counted.
*/
void tests_spend(const test_api_t *api, uint64_t units)
{
    backend_instance_t *i = tests_get_instance(api);

    if (!i->bounded)
        return;
    if (units > i->work) {
        i->work = 0;
        end_call(i, "the work limit is used up");
    }
    i->work -= units;
}

/*
** tests_wait
**
** Counts nanoseconds against the timeout of this call, and ends the call
** past it. With none set, it returns: no time passes either way.
*/
void tests_wait(const test_api_t *api, uint64_t nanos)
{
    backend_instance_t *i = tests_get_instance(api);

    if (i->timeout && nanos > i->timeout)
        end_call(i, "the timeout passed");
}

/*
** tests_grow
**
** Asks for bytes of memory of the module's own: 1, or 0 past the limit.
** Nothing is allocated: what a module has is counted.
*/
int tests_grow(const test_api_t *api, uint64_t bytes)
{
    backend_instance_t *i = tests_get_instance(api);

    if (i->memory_limit && bytes > i->memory_limit - i->memory)
        return (0);
    i->memory += bytes;
    return (1);
}

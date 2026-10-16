/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "tests_inside.h"

static void end_call(backend_instance_t *i, const char *why)
{
    if (!i->out)
        return;
    i->why = why;
    longjmp(*i->out, 1);
}

void tests_trap(const test_api_t *api)
{
    end_call(tests_get_instance(api), "the module asked for it");
}

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

void tests_wait(const test_api_t *api, uint64_t nanos)
{
    backend_instance_t *i = tests_get_instance(api);

    if (i->timeout && nanos > i->timeout)
        end_call(i, "the timeout passed");
}

int tests_grow(const test_api_t *api, uint64_t bytes)
{
    backend_instance_t *i = tests_get_instance(api);

    if (i->memory_limit && bytes > i->memory_limit - i->memory)
        return (0);
    i->memory += bytes;
    return (1);
}

/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/reference/flecs_ref.c -- flecs as the reference: the measures, in
** order.
**
** stage11.md, *Beside the best there is*
*/

#include <stdio.h>

#include "flecs_ref.h"

/*
** main
**
** Makes each measure, a change both at once and deferred. Nothing is
** buffered: a crash must not swallow what was said. The draw is one
** sequence for the whole run.
*/
int main(void)
{
    uint32_t state = 2463534242u;
    uint32_t k = 0;

    setvbuf(stdout, nullptr, _IONBF, 0);
    flecs_ref_measure_basics(false);
    flecs_ref_measure_basics(true);
    while (k < 8) {
        flecs_ref_measure_change(k & 1, k >> 1 & 1, k >> 2 & 1);
        k++;
    }
    flecs_ref_measure_reads();
    flecs_ref_measure_reach(&state, false);
    flecs_ref_measure_reach(&state, true);
    flecs_ref_measure_toggle(&state, 1);
    flecs_ref_measure_toggle(&state, 2);
    flecs_ref_measure_toggle(&state, 16);
    flecs_ref_measure_singleton();
    flecs_ref_measure_markers(false);
    flecs_ref_measure_markers(true);
    return (0);
}

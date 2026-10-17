/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "abi_entry.h"
#include "abi_subscribe.h"

static int read_as_it_is(module_t *m, const char *at, uint32_t len,
    uint32_t *phase)
{
    const bridge_t *b = m->b;
    int shown = len < 40 ? (int)len : 40;
    uint32_t i = 0;

    *phase = bridge_find_phase(b->phases, b->phase_count, at, len);
    if (*phase == b->phase_count)
        return (abi_refuse(m, "a system names phase '%.*s', which the "
            "deployment did not declare", shown, at));
    while (i < m->phase_names && m->phase_to[i] != *phase)
        i++;
    if (i == m->phase_names)
        return (0);
    return (abi_refuse(m, "a system names phase '%.*s' as it is, and its "
        "config also maps '%s' to that phase", shown, at,
        m->phase_from[i]));
}

int abi_read_phase(module_t *m, const bridge_system_statement_t *st,
    uint32_t *phase)
{
    const char *at = get_scratch(m, st->phase, st->phase_len);
    uint32_t i;

    if (!at)
        return (abi_refuse(m, "add_system: a phase lies outside the scratch "
            "arena"));
    i = bridge_find_phase(m->phase_from, m->phase_names, at, st->phase_len);
    if (i < m->phase_names) {
        *phase = m->phase_to[i];
        return (0);
    }
    return (read_as_it_is(m, at, st->phase_len, phase));
}

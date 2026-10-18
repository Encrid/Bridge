/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "nsparse.h"
#include "native_idle.h"

static void add_changes(const bridge_api_t *api)
{
    int none = 0;

    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, &nsparse_a, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_b", "attach_b", &none, 0,
        &nsparse_s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0,
        &nsparse_s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0,
        &nsparse_s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "value_b", "value_b", &none, 0, &nsparse_s,
        1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_b_half", "attach_b_half", &none, 0,
        &nsparse_s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy_half", "destroy_half", &none, 0,
        nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy", "destroy", &none, 0, nullptr, 0);
}

static void add_walks(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ nsparse_s, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ nsparse_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { nsparse_s, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    nsparse_in_set = BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", one, 1,
        nullptr, 0);
    nsparse_in_table = BRIDGE_NATIVE_ADD_SYSTEM(api, "look_b", "look_b", two,
        2, nullptr, 0);
}

#if !defined(BIG) && !defined(FAT)

static void add_observers(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ nsparse_s, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ nsparse_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { nsparse_s, BRIDGE_READ, BRIDGE_AND, 0 }};

    nsparse_on_s = BRIDGE_NATIVE_OBSERVER(api, "heard_s",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
    nsparse_on_both = BRIDGE_NATIVE_OBSERVER(api, "heard_both",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr,
        0);
}

#endif

#ifdef BAD

static void add_refused(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ nsparse_s, BRIDGE_READ, BRIDGE_NOT, 0 }};
    bridge_term_t two[2] = {{ nsparse_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { nsparse_s, BRIDGE_READ, BRIDGE_AND, 0 }};

    if (BAD == 1)
        BRIDGE_NATIVE_ADD_SYSTEM(api, "both", "never", two, 2, nullptr, 0);
    else
        BRIDGE_NATIVE_ADD_SYSTEM(api, "without", "never", one, 1, nullptr, 0);
}

#endif

BRIDGE_NATIVE(init)
{
    nsparse_a = BRIDGE_NATIVE_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    nsparse_s = BRIDGE_NATIVE_COMPONENT(api, "st:S", s_t, BRIDGE_DELETE,
        BRIDGE_SPARSE_SET);
    add_changes(api);
    add_walks(api);
#if !defined(BIG) && !defined(FAT)
    add_observers(api);
#endif
#ifdef BAD
    add_refused(api);
#endif
    return (0);
}

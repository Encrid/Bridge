/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "offsets_idle.h"

BRIDGE_EXPORT(work) int work(void)
{
    return (0);
}

BRIDGE_EXPORT(churn) int churn(void)
{
    bridge_destroy(bridge_spawn());
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("work", "work", &none, 0, nullptr, 0, nullptr, 0);
    BRIDGE_ADD_SYSTEM("churn", "churn", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}

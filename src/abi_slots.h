/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_ABI_SLOTS_H
    #define BRIDGE_SRC_ABI_SLOTS_H

    #include <string.h>

    #include "abi_entry.h"

static inline const char *get_slots(const module_t *m, uint32_t at,
    uint32_t count)
{
    if (!count)
        return ("");
    return (get_scratch(m, at, (uint64_t)count * sizeof(uint32_t)));
}

static inline uint32_t get_slot(const char *slots, uint32_t i)
{
    uint32_t slot;

    memcpy(&slot, slots + (size_t)i * sizeof(slot), sizeof(slot));
    return (slot);
}

#endif

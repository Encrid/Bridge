/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_WASM_OBSERVER_H
    #define BRIDGE_WASM_OBSERVER_H

    #include "bridge_wasm_acc.h"

static inline uint32_t bridge_count_triggers(uint32_t observer)
{
    uint32_t at = BRIDGE_HEADER(observers);

    return (acc_own_32(at + observer * sizeof(bridge_list_t)));
}

static inline uint32_t bridge_get_trigger(uint32_t observer, uint32_t i)
{
    uint32_t at = BRIDGE_HEADER(observers) + observer * sizeof(bridge_list_t);

    return (acc_own_32(at + 4u) + i * (uint32_t)sizeof(bridge_trigger_t));
}

static inline uint64_t bridge_get_trigger_id(uint32_t trigger)
{
    return (acc_own_64(trigger));
}

static inline uint32_t bridge_get_trigger_events(uint32_t trigger)
{
    return (acc_own_32(trigger + 8u));
}

#endif

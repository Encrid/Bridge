/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** backends/wasm/sdk/bridge_wasm_observer.h -- an observer's list: who
** triggered it at this barrier, and how.
**
** What is compiled with this file is a work based on Bridge and is under
** the same licence. Other terms: docs/licence.md.
**
** query.md, *The interface is a query*
*/

#ifndef BRIDGE_WASM_OBSERVER_H
    #define BRIDGE_WASM_OBSERVER_H

    #include "bridge_wasm_acc.h"

/*
** bridge_count_triggers
**
** Answers how many triggered an observer at this barrier.
**
** data.md, *Observers*
*/
static inline uint32_t bridge_count_triggers(uint32_t observer)
{
    uint32_t at = BRIDGE_HEADER(observers);

    return (acc_own_32(at + observer * sizeof(bridge_list_t)));
}

/*
** bridge_get_trigger
**
** Answers trigger `i` of an observer.
**
** data.md, *Observers*
*/
static inline uint32_t bridge_get_trigger(uint32_t observer, uint32_t i)
{
    uint32_t at = BRIDGE_HEADER(observers) + observer * sizeof(bridge_list_t);

    return (acc_own_32(at + 4u) + i * (uint32_t)sizeof(bridge_trigger_t));
}

/*
** bridge_get_trigger_id
**
** Answers the entity a trigger is of.
**
** data.md, *Observers*
*/
static inline uint64_t bridge_get_trigger_id(uint32_t trigger)
{
    return (acc_own_64(trigger));
}

/*
** bridge_get_trigger_events
**
** Answers what a trigger's entity became, as a mask of bridge_event.
**
** data.md, *Observers*
*/
static inline uint32_t bridge_get_trigger_events(uint32_t trigger)
{
    return (acc_own_32(trigger + 8u));
}

#endif

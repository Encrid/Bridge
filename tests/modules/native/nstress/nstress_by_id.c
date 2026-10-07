/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nstress/nstress_by_id.c -- the account nstress
** keeps, and every entity checked against it by its id.
*/

#include "nstress.h"
#include "native/native_reach.h"

uint64_t nstress_ids[SLOTS];
uint8_t nstress_has[SLOTS];
uint32_t nstress_values[SLOTS][KINDS];
uint32_t nstress_wrong;
static uint32_t passes;

/*
** check_kinds
**
** Checks each component of the entity of `slot`, which `record` says where
** to find: that its table carries the component exactly when the account
** says so, and that the value is the account's. Answers `digest` with the
** values the account has for it.
*/
static uint32_t check_kinds(const bridge_api_t *api,
    const bridge_entity_t *record, uint32_t slot, uint32_t digest)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    const uint32_t *column_base = get_column_bases(api);
    uint32_t base;
    uint32_t k = 0;

    while (k < KINDS) {
        base = column_base[record->table * h->types + nstress_kinds[k]];
        if (!base != !(nstress_has[slot] >> k & 1)) {
            nstress_wrong++;
        } else if (base) {
            nstress_wrong += ((const v_t *)(api->shared_base
                + base))[record->row].v != nstress_values[slot][k];
            digest = digest * 31 + nstress_values[slot][k];
        }
        k++;
    }
    return (digest);
}

/*
** check_slot
**
** Checks the entity of `slot`: that its record of the index has the
** generation of its id, and then each of its components. Answers `digest`
** with what the account has for it.
*/
static uint32_t check_slot(const bridge_api_t *api,
    const bridge_entity_t *record, uint32_t slot, uint32_t digest)
{
    digest = digest * 31 + nstress_has[slot];
    if (record->generation != (uint32_t)(nstress_ids[slot] >> 32)) {
        nstress_wrong++;
        return (digest);
    }
    return (check_kinds(api, record, slot, digest));
}

static void say_account(const bridge_api_t *api, uint32_t live,
    uint32_t digest)
{
    bridge_line_t account = { "live, digest", 12, live, digest };
    bridge_line_t wrong = { "disagreements so far", 20, nstress_wrong, 0 };

    bridge_native_log(api, BRIDGE_LOG_INFO, &account);
    bridge_native_log(api, BRIDGE_LOG_INFO, &wrong);
}

/*
** nstress_check_entities
**
** Looks at every entity by its id: where the index says it is, and what
** column_base says is there. Counts what disagrees with the account, and
** every twenty-fifth time says how many entities are alive, a digest of the
** account, and the disagreements so far.
*/
void nstress_check_entities(const bridge_api_t *api)
{
    const bridge_entity_t *index = get_entity_index(api);
    uint32_t live = 0;
    uint32_t digest = 0;
    uint32_t slot = 0;

    while (slot < SLOTS) {
        if (nstress_ids[slot]) {
            live++;
            digest = check_slot(api, &index[(uint32_t)nstress_ids[slot]],
                slot, digest);
        }
        slot++;
    }
    passes++;
    if (passes % 25 == 0)
        say_account(api, live, digest);
}

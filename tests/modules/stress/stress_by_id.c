/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "stress.h"
#include "reach.h"

uint64_t nstress_ids[SLOTS];
uint8_t nstress_has[SLOTS];
uint32_t nstress_values[SLOTS][KINDS];
uint32_t nstress_wrong;
static uint32_t passes;

static uint32_t check_kinds(const test_api_t *api,
    const bridge_entity_t *record, uint32_t slot, uint32_t digest)
{
    const bridge_header_t *h = test_get_header(api);
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

static uint32_t check_slot(const test_api_t *api,
    const bridge_entity_t *record, uint32_t slot, uint32_t digest)
{
    digest = digest * 31 + nstress_has[slot];
    if (record->generation != (uint32_t)(nstress_ids[slot] >> 32)) {
        nstress_wrong++;
        return (digest);
    }
    return (check_kinds(api, record, slot, digest));
}

static void say_account(const test_api_t *api, uint32_t live,
    uint32_t digest)
{
    bridge_line_t account = { "live, digest", 12, live, digest };
    bridge_line_t wrong = { "disagreements so far", 20, nstress_wrong, 0 };

    test_log(api, BRIDGE_LOG_INFO, &account);
    test_log(api, BRIDGE_LOG_INFO, &wrong);
}

void nstress_check_entities(const test_api_t *api)
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

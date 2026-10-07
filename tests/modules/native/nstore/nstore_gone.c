/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/native/nstore/nstore_gone.c -- entities of the plain build
** of nstore destroyed and asked for again, and what becomes of the ids
** that went.
*/

#include "nstore.h"
#include "native/native_reach.h"

#if !defined(BIG) && !defined(TABLES)
constexpr uint32_t KEPT = 64;

uint64_t nstore_old[COUNT];
static uint64_t gone[KEPT];
static uint32_t gone_count;
static uint32_t reused;

/*
** destroy_half
**
** Destroys the second half of the entities, each twice, and keeps their
** ids.
*/
BRIDGE_NATIVE(destroy_half)
{
    uint32_t i = COUNT / 2;

    while (i < COUNT) {
        nstore_old[i] = nstore_ids[i];
        if (gone_count < KEPT) {
            gone[gone_count] = nstore_ids[i];
            gone_count++;
        }
        api->destroy(api, nstore_ids[i]);
        api->destroy(api, nstore_ids[i]);
        i++;
    }
    return (0);
}

static bool is_alive(const bridge_api_t *api, uint64_t id)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    const bridge_entity_t *index = get_entity_index(api);

    return ((uint32_t)id < h->entities
        && index[(uint32_t)id].generation == (uint32_t)(id >> 32));
}

/*
** count_whole
**
** Answers how many destroyed ids `id` is the same as, generation and all,
** which must be none: an index a destroyed id had is one of the module's
** again, and never under the same generation. Notes that an index was used
** again.
*/
static uint32_t count_whole(uint64_t id)
{
    uint32_t same = 0;
    uint32_t k = 0;

    while (k < gone_count) {
        if ((uint32_t)id == (uint32_t)gone[k]) {
            reused = 1;
            same += id == gone[k];
        }
        k++;
    }
    return (same);
}

/*
** respawn_half
**
** Spawns the second half of the entities again, and says how many of the
** destroyed ids read as dead, and how many ids came back whole.
*/
BRIDGE_NATIVE(respawn_half)
{
    uint32_t same = 0;
    uint32_t dead = 0;
    uint32_t i = COUNT / 2;
    uint32_t k = 0;

    while (i < COUNT) {
        nstore_ids[i] = api->spawn(api);
        same += count_whole(nstore_ids[i]);
        i++;
    }
    while (k < gone_count) {
        dead += !is_alive(api, gone[k]);
        k++;
    }
    say(api, "destroyed ids that read as dead, of", dead, gone_count);
    say(api, "ids that came back whole", same, 0);
    return (0);
}

/*
** recycled
**
** Says whether an index was used again. When a freed index comes back is
** storage's own business; that one does, in the end, is not.
*/
BRIDGE_NATIVE(recycled)
{
    say(api, "an index was used again", reused, 0);
    return (0);
}

#endif

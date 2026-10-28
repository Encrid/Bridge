/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "husk.h"

uint32_t husk_frame;
uint64_t husk_ids[3];

static void make_three(void)
{
    body_t b = { 1 };
    handle_t k[3] = {{ 7 }, { 5 }, { 3 }};
    uint32_t i = 0;

    while (i < 3) {
        husk_ids[i] = bridge_spawn();
        BRIDGE_ATTACH(husk_ids[i], husk_body, b);
        BRIDGE_ATTACH(husk_ids[i], husk_handle, k[i]);
        i++;
    }
    bridge_destroy(husk_ids[1]);
}

static void destroy_two(void)
{
    handle_t before = { 4 };
    handle_t after = { 9 };

    BRIDGE_ATTACH(husk_ids[2], husk_handle, before);
#ifndef LIVE
    bridge_destroy(husk_ids[0]);
    bridge_destroy(husk_ids[2]);
#endif
    BRIDGE_ATTACH(husk_ids[2], husk_handle, after);
}

static void touch_first(uint32_t frame)
{
    body_t two = { 2 };

    if (frame == 3)
        BRIDGE_ATTACH(husk_ids[0], husk_body, two);
    if (frame == 4)
        bridge_destroy(husk_ids[0]);
    if (frame == 5)
        bridge_detach(husk_ids[0], husk_handle);
}

BRIDGE_EXPORT(step) int step(void)
{
    husk_frame++;
    if (husk_frame == 1)
        make_three();
    else if (husk_frame == 2)
        destroy_two();
    else
        touch_first(husk_frame);
    return (0);
}

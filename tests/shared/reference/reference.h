/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_REFERENCE_H
    #define TEST_REFERENCE_H

    #include <stdint.h>
    #include <time.h>

static inline uint64_t read_clock(void)
{
    struct timespec t;

    timespec_get(&t, TIME_UTC);
    return ((uint64_t)t.tv_sec * 1'000'000'000u + (uint64_t)t.tv_nsec);
}

static inline uint32_t draw_random(uint32_t *state)
{
    *state ^= *state << 13;
    *state ^= *state >> 17;
    *state ^= *state << 5;
    return (*state);
}

#endif

/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/reference/reference.h -- what the reference programs share: the
** clock a region is timed with, and the draw that puts ids in no order.
**
** Built by tools/reference.py. Not part of the library, and not built by
** build.py.
*/

#ifndef TEST_REFERENCE_H
    #define TEST_REFERENCE_H

    #include <stdint.h>
    #include <time.h>

/*
** read_clock
**
** Answers the time, in nanoseconds.
*/
static inline uint64_t read_clock(void)
{
    struct timespec t;

    timespec_get(&t, TIME_UTC);
    return ((uint64_t)t.tv_sec * 1'000'000'000u + (uint64_t)t.tv_nsec);
}

/*
** draw_random
**
** Answers the next number of a sequence that is the same on every run, and
** moves `state` on. The first state is 2463534242.
*/
static inline uint32_t draw_random(uint32_t *state)
{
    *state ^= *state << 13;
    *state ^= *state >> 17;
    *state ^= *state << 5;
    return (*state);
}

#endif

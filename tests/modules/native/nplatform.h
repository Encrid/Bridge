/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/native/nplatform.h -- what a native test module asks the
** operating system and is spelled differently on Windows: a sleep, the
** thread it is on, and the time.
*/

#ifndef TEST_NPLATFORM_H
    #define TEST_NPLATFORM_H

    #include <stdint.h>

    #ifdef _WIN32
        #define WIN32_LEAN_AND_MEAN
        #include <windows.h>
    #else
        #include <pthread.h>
        #include <time.h>
        #include <unistd.h>
    #endif

/*
** sleep_tenth
**
** Sleeps a tenth of a second, or more: a sleep is a least time.
*/
static inline void sleep_tenth(void)
{
    #ifdef _WIN32
    Sleep(100);
    #else
    usleep(100'000);
    #endif
}

/*
** get_thread_id
**
** Answers a number that is the calling thread's and no other running
** thread's.
*/
static inline uint32_t get_thread_id(void)
{
    #ifdef _WIN32
    return ((uint32_t)GetCurrentThreadId());
    #else
    return ((uint32_t)(uintptr_t)pthread_self());
    #endif
}

    #ifdef _WIN32

/*
** get_now_ms
**
** Answers a time in milliseconds, from a clock that does not go back.
*/
static inline uint32_t get_now_ms(void)
{
    return ((uint32_t)GetTickCount64());
}

    #else

/*
** get_now_ms
**
** Answers a time in milliseconds, from a clock that does not go back.
*/
static inline uint32_t get_now_ms(void)
{
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);
    return ((uint32_t)((uint64_t)t.tv_sec * 1000
        + (uint64_t)t.tv_nsec / 1'000'000));
}

    #endif

#endif

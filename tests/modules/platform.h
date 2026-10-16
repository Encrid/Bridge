/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_PLATFORM_H
    #define TEST_PLATFORM_H

    #include <stdint.h>

    #ifdef _WIN32
        #define WIN32_LEAN_AND_MEAN
        #include <windows.h>
    #else
        #include <pthread.h>
        #include <time.h>
        #include <unistd.h>
    #endif

static inline void sleep_tenth(void)
{
    #ifdef _WIN32
    Sleep(100);
    #else
    usleep(100'000);
    #endif
}

static inline uint32_t get_thread_id(void)
{
    #ifdef _WIN32
    return ((uint32_t)GetCurrentThreadId());
    #else
    return ((uint32_t)(uintptr_t)pthread_self());
    #endif
}

    #ifdef _WIN32

static inline uint32_t get_now_ms(void)
{
    return ((uint32_t)GetTickCount64());
}

    #else

static inline uint32_t get_now_ms(void)
{
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);
    return ((uint32_t)((uint64_t)t.tv_sec * 1000
        + (uint64_t)t.tv_nsec / 1'000'000));
}

    #endif

#endif

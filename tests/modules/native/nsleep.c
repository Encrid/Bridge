/* A system that takes a tenth of a second or more, and says which thread it was on, when it
 * began and when it ended. Two of these whose times overlap ran at once. */
#include "bridge-native.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define nap() Sleep(100)
#define thread_id() ((uint32_t)GetCurrentThreadId())
#define now_ms() ((uint32_t)GetTickCount64())
#else
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#define nap() usleep(100'000)
#define thread_id() ((uint32_t)(uintptr_t)pthread_self())
static uint32_t now_ms()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint32_t)((uint64_t)t.tv_sec * 1000 + (uint64_t)t.tv_nsec / 1'000'000);
}
#endif

static const bridge_api *api;

BRIDGE_NATIVE_BIND(api)

BRIDGE_NATIVE(nap)
{
    /* When it began and when it ended, in milliseconds: two ran at once if one began before
     * the other ended. How long a nap takes is not asked: a sleep is a least time, and some
     * systems return from a tenth of a second after two. */
    uint32_t began = now_ms();
    nap();
    bridge_native_log(api, BRIDGE_LOG_INFO, "nap", 3, thread_id(), began);
    bridge_native_log(api, BRIDGE_LOG_INFO, "end", 3, thread_id(), now_ms());
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0;
    BRIDGE_NATIVE_SYSTEM(api, "nap", "update", &none, 0);
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }

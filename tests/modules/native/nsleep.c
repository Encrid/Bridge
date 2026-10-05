/* A system that takes a tenth of a second and says which thread it was on. Two of these that
 * finish in under two tenths ran at once. */
#include "bridge-native.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define nap() Sleep(100)
#define thread_id() ((uint32_t)GetCurrentThreadId())
#else
#include <pthread.h>
#include <unistd.h>
#define nap() usleep(100000)
#define thread_id() ((uint32_t)(uintptr_t)pthread_self())
#endif

static const bridge_api *api;

BRIDGE_NATIVE_BIND(api)

BRIDGE_NATIVE(nap)
{
    nap();
    bridge_native_log(api, 0, "nap", 3, thread_id(), 0);
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0;
    BRIDGE_NATIVE_SYSTEM(api, "nap", "update", &none, 0);
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }

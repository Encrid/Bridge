/* Three systems that each say which thread they were on. Loaded pinned, it must be one. */
#include "bridge-native.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define thread_id() ((uint32_t)GetCurrentThreadId())
#else
#include <pthread.h>
#define thread_id() ((uint32_t)(uintptr_t)pthread_self())
#endif



static int pin(const bridge_api_t *api)
{
    bridge_native_log(api, BRIDGE_LOG_INFO, "pin", 3, thread_id(), 0);
    return 0;
}

BRIDGE_NATIVE(one)   { return pin(api); }
BRIDGE_NATIVE(two)   { return pin(api); }
BRIDGE_NATIVE(three) { return pin(api); }

BRIDGE_NATIVE(init)
{
    int none = 0;
    BRIDGE_NATIVE_SYSTEM(api, "one", "update", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "two", "update", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "three", "present", &none, 0, nullptr, 0);
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }

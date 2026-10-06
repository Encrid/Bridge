/* An entry reached from a thread the backend did not call on answers zero, and one reached on
 * the thread it did call on is honoured: docs/backends/native/native.md, *The invariant*.
 *
 * Its one system keeps the table it was handed, asks another thread to spawn through it, waits
 * for that thread, and then spawns itself. It says whether each got an id. */
#include "bridge_native.h"

static const bridge_api_t *kept;
static uint64_t from_elsewhere;

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static DWORD WINAPI elsewhere([[maybe_unused]] LPVOID unused)
{
    from_elsewhere = kept->spawn(kept);
    return 0;
}
static void on_another_thread()
{
    HANDLE t = CreateThread(nullptr, 0, elsewhere, nullptr, 0, nullptr);
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
}
#else
#include <pthread.h>
static void *elsewhere([[maybe_unused]] void *unused)
{
    from_elsewhere = kept->spawn(kept);
    return nullptr;
}
static void on_another_thread()
{
    pthread_t t;
    pthread_create(&t, nullptr, elsewhere, nullptr);
    pthread_join(t, nullptr);
}
#endif

BRIDGE_NATIVE(reach)
{
    uint64_t here;
    kept = api;
    on_another_thread();
    here = api->spawn(api);
    bridge_native_log(api, BRIDGE_LOG_INFO, "an id from another thread, and from this one", 44,
        from_elsewhere != 0, here != 0);
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0;
    BRIDGE_NATIVE_ADD_SYSTEM(api, "reach", "update", &none, 0, nullptr, 0);
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }

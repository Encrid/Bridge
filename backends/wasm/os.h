/* The operating system, as far as the wasm backend needs it: address space for a memory, a
 * thread for its watchdog, an event, a mutex and a clock. It is this backend's own: nothing of
 * Bridge-ASM700's inside is compiled into a backend. */
#ifndef BRIDGE_WASM_OS_H
#define BRIDGE_WASM_OS_H

#include <stddef.h>
#include <stdint.h>

/* Reserve `reserve` bytes of address space and make the first `commit` of them
 * readable, writable and zero. The rest faults when touched, which is what a
 * backend that enforces bounds by faulting needs after a block. nullptr on failure. */
[[nodiscard]] void *os_reserve(size_t reserve, size_t commit);
[[nodiscard]] int os_commit(void *base, size_t bytes);          /* 0 on success */
void  os_release(void *base, size_t reserve);

typedef struct os_thread os_thread_t;
[[nodiscard]] os_thread_t *os_thread_start(void (*fn)(void *), void *arg);
void       os_thread_join(os_thread_t *t);
uint64_t   os_get_now();                            /* nanoseconds, from a clock that only rises */

typedef struct os_mutex os_mutex_t;
[[nodiscard]] os_mutex_t *os_mutex_create();
void      os_mutex_free(os_mutex_t *m);
void      os_mutex_lock(os_mutex_t *m);
void      os_mutex_unlock(os_mutex_t *m);

/* An event one thread sets and another waits for. Setting it before the wait
 * is not lost; a wait consumes it. */
typedef struct os_event os_event_t;
constexpr uint64_t OS_FOREVER = UINT64_MAX;
[[nodiscard]] os_event_t *os_event_create();
void      os_event_free(os_event_t *e);
void      os_event_set(os_event_t *e);
int       os_event_wait(os_event_t *e, uint64_t nanos);   /* 1 if set, 0 if it timed out */

#endif

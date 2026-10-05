/* The operating system, as far as Bridge-ASM700 and its backends need it:
 * address space, shared libraries, threads, an event and a mutex. Nothing here knows
 * what a module is. */
#ifndef BRIDGE_OS_H
#define BRIDGE_OS_H

#include <stddef.h>
#include <stdint.h>

/* Reserve `reserve` bytes of address space and make the first `commit` of them
 * readable, writable and zero. The rest faults when touched, which is what a
 * backend that enforces bounds by faulting needs after a block. nullptr on failure. */
void *os_reserve(size_t reserve, size_t commit);
int   os_commit(void *base, size_t bytes);          /* 0 on success */
void  os_release(void *base, size_t reserve);

void *os_lib_open(const char *path);                /* nullptr on failure */
void *os_lib_sym(void *lib, const char *name);
void  os_lib_close(void *lib);

typedef struct os_thread os_thread;
os_thread *os_thread_start(void (*fn)(void *), void *arg);
void       os_thread_join(os_thread *t);
uint64_t   os_thread_id();                      /* of the calling thread */
uint64_t   os_now();                            /* nanoseconds, from a clock that only rises */

/* One lock for the process, for what a library can only do one at a time. */
void os_global_lock();
void os_global_unlock();

typedef struct os_mutex os_mutex;
os_mutex *os_mutex_new();
void      os_mutex_free(os_mutex *m);
void      os_mutex_lock(os_mutex *m);
void      os_mutex_unlock(os_mutex *m);

/* An event one thread sets and another waits for. Setting it before the wait
 * is not lost; a wait consumes it. */
typedef struct os_event os_event;
constexpr uint64_t OS_FOREVER = UINT64_MAX;
os_event *os_event_new();
void      os_event_free(os_event *e);
void      os_event_set(os_event *e);
int       os_event_wait(os_event *e, uint64_t nanos);   /* 1 if set, 0 if it timed out */

#endif

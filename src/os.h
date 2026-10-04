/* The operating system, as far as Bridge-ASM700 and its backends need it:
 * address space, shared libraries, one thread and one event. Nothing here knows
 * what a module is. */
#ifndef BRIDGE_OS_H
#define BRIDGE_OS_H

#include <stddef.h>
#include <stdint.h>

/* Reserve `reserve` bytes of address space and make the first `commit` of them
 * readable, writable and zero. The rest faults when touched, which is what a
 * backend that enforces bounds by faulting needs after a block. NULL on failure. */
void *os_reserve(size_t reserve, size_t commit);
int   os_commit(void *base, size_t bytes);          /* 0 on success */
void  os_release(void *base, size_t reserve);

void *os_lib_open(const char *path);                /* NULL on failure */
void *os_lib_sym(void *lib, const char *name);
void  os_lib_close(void *lib);

typedef struct os_thread os_thread;
os_thread *os_thread_start(void (*fn)(void *), void *arg);
void       os_thread_join(os_thread *t);

/* An event one thread sets and another waits for. Setting it before the wait
 * is not lost; a wait consumes it. */
typedef struct os_event os_event;
#define OS_FOREVER UINT64_MAX
os_event *os_event_new(void);
void      os_event_free(os_event *e);
void      os_event_set(os_event *e);
int       os_event_wait(os_event *e, uint64_t nanos);   /* 1 if set, 0 if it timed out */

#endif

/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** utils/os/os.h -- the operating system, as far as Bridge and its
** backends need it: address space, shared libraries, threads, the clock, a
** mutex and an event.
**
** It is the only code that differs by platform, and it depends on nothing of
** ours: the library compiles it, and so does each backend, which is a binary
** of its own. What is written once for every platform is here, and each
** platform's folder holds the same functions for it.
**
** build.md, *The C library*
*/

#ifndef BRIDGE_OS_H
    #define BRIDGE_OS_H

    #include <stddef.h>
    #include <stdint.h>

typedef struct os_thread os_thread_t;
typedef struct os_mutex os_mutex_t;
typedef struct os_event os_event_t;
typedef void (*os_thread_fn_t)(void *arg);

/*
** How long os_event_wait waits when it is to wait until the event is set.
*/
constexpr uint64_t OS_FOREVER = UINT64_MAX;

/*
** Nanoseconds in a second, and in a millisecond.
*/
constexpr uint64_t OS_SECOND = 1'000'000'000;
constexpr uint64_t OS_MILLISECOND = 1'000'000;

/*
** os_reserve
**
** Reserves `reserve` bytes of address space and makes the first `commit` of
** them readable, writable and zero. The rest faults when touched, which is
** what a backend that enforces bounds by faulting needs after a block.
** nullptr on failure.
*/
[[nodiscard]] void *os_reserve(size_t reserve, size_t commit);

/*
** os_commit
**
** Makes the first `bytes` of a reservation readable and writable. 0 on
** success.
*/
[[nodiscard]] int os_commit(void *base, size_t bytes);

/*
** os_release
**
** Gives a reservation back, all of it. A null `base` is nothing to give.
*/
void os_release(void *base, size_t reserve);

/*
** os_lib_open
**
** Opens a shared library. One named by a path is looked for there, and so is
** what it needs: a backend brings its runtime beside itself, wherever the
** host's own executable is. A bare name is left to the operating system's
** search. nullptr on failure.
*/
[[nodiscard]] void *os_lib_open(const char *path);

/*
** os_lib_get_symbol
**
** Answers where a library has what it exports by `name`, or nullptr.
*/
[[nodiscard]] void *os_lib_get_symbol(void *lib, const char *name);

/*
** os_lib_close
**
** Closes a library. A null `lib` is nothing to close.
*/
void os_lib_close(void *lib);

/*
** os_thread_start
**
** Starts a thread that runs `fn(arg)`. nullptr on failure.
*/
[[nodiscard]] os_thread_t *os_thread_start(os_thread_fn_t fn, void *arg);

/*
** os_thread_join
**
** Waits for a thread to end, and frees what os_thread_start answered.
*/
void os_thread_join(os_thread_t *t);

/*
** os_get_thread_id
**
** Answers a number for the calling thread that no other live thread has. It
** is never 0.
*/
uint64_t os_get_thread_id(void);

/*
** os_get_now
**
** Answers nanoseconds, from a clock that only rises.
*/
uint64_t os_get_now(void);

/*
** os_mutex_create
**
** Makes a mutex. nullptr on failure.
*/
[[nodiscard]] os_mutex_t *os_mutex_create(void);

/*
** os_mutex_free
**
** Frees a mutex nobody holds. A null `m` is nothing to free.
*/
void os_mutex_free(os_mutex_t *m);

/*
** os_mutex_lock
**
** Takes a mutex, waiting for whoever holds it.
*/
void os_mutex_lock(os_mutex_t *m);

/*
** os_mutex_unlock
**
** Gives a mutex back.
*/
void os_mutex_unlock(os_mutex_t *m);

/*
** os_event_create
**
** Makes an event one thread sets and another waits for. Setting it before the
** wait is not lost; a wait consumes it. nullptr on failure.
*/
[[nodiscard]] os_event_t *os_event_create(void);

/*
** os_event_free
**
** Frees an event nobody waits for. A null `e` is nothing to free.
*/
void os_event_free(os_event_t *e);

/*
** os_event_set
**
** Sets an event: one wait, now or later, returns for it.
*/
void os_event_set(os_event_t *e);

/*
** os_event_wait
**
** Waits for an event for at most `nanos`, or until it is set if that is
** OS_FOREVER. 1 if it was set, 0 if the time ran out.
*/
int os_event_wait(os_event_t *e, uint64_t nanos);

#endif

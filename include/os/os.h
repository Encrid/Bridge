/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_OS_H
    #define BRIDGE_OS_H

    #include <stddef.h>
    #include <stdint.h>

typedef struct os_thread os_thread_t;
typedef struct os_mutex os_mutex_t;
typedef struct os_event os_event_t;
typedef void (*os_thread_fn_t)(void *arg);

constexpr uint64_t OS_SECOND = 1'000'000'000;
constexpr uint64_t OS_FOREVER = UINT64_MAX;

[[nodiscard]] void *os_reserve(size_t reserve, size_t commit);

[[nodiscard]] int os_commit(void *base, size_t bytes);

void os_release(void *base, size_t reserve);

[[nodiscard]] void *os_lib_open(const char *path);

[[nodiscard]] void *os_lib_get_symbol(void *lib, const char *name);

void os_lib_close(void *lib);

[[nodiscard]] os_thread_t *os_thread_start(os_thread_fn_t fn, void *arg);

void os_thread_join(os_thread_t *t);

uint64_t os_get_thread_id(void);

uint64_t os_get_now(void);

[[nodiscard]] os_mutex_t *os_mutex_create(void);

void os_mutex_free(os_mutex_t *m);

void os_mutex_lock(os_mutex_t *m);

void os_mutex_unlock(os_mutex_t *m);

[[nodiscard]] os_event_t *os_event_create(void);

void os_event_free(os_event_t *e);

void os_event_set(os_event_t *e);

void os_event_wait(os_event_t *e, uint64_t nanos);

[[nodiscard]] int os_folder_set(const char *folder);

[[nodiscard]] int os_folder_make(const char *folder);

[[nodiscard]] char *os_path_whole(const char *path);

[[nodiscard]] int os_lock(const char *path);

#endif

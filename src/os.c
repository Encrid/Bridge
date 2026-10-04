#include "os.h"

#include <stdlib.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void *os_reserve(size_t reserve, size_t commit)
{
    void *base = VirtualAlloc(NULL, reserve, MEM_RESERVE, PAGE_NOACCESS);
    if (base && commit && !VirtualAlloc(base, commit, MEM_COMMIT, PAGE_READWRITE)) {
        VirtualFree(base, 0, MEM_RELEASE);
        return NULL;
    }
    return base;
}

int os_commit(void *base, size_t bytes)
{
    return VirtualAlloc(base, bytes, MEM_COMMIT, PAGE_READWRITE) ? 0 : -1;
}

void os_release(void *base, size_t reserve)
{
    (void)reserve;
    if (base) VirtualFree(base, 0, MEM_RELEASE);
}

void *os_lib_open(const char *path) { return (void *)LoadLibraryA(path); }
void *os_lib_sym(void *lib, const char *name) { return (void *)GetProcAddress((HMODULE)lib, name); }
void  os_lib_close(void *lib) { if (lib) FreeLibrary((HMODULE)lib); }

struct os_thread { HANDLE handle; void (*fn)(void *); void *arg; };

static DWORD WINAPI thread_main(LPVOID p)
{
    os_thread *t = p;
    t->fn(t->arg);
    return 0;
}

os_thread *os_thread_start(void (*fn)(void *), void *arg)
{
    os_thread *t = malloc(sizeof *t);
    t->fn = fn;
    t->arg = arg;
    t->handle = CreateThread(NULL, 0, thread_main, t, 0, NULL);
    return t;
}

void os_thread_join(os_thread *t)
{
    WaitForSingleObject(t->handle, INFINITE);
    CloseHandle(t->handle);
    free(t);
}

struct os_event { HANDLE handle; };

os_event *os_event_new(void)
{
    os_event *e = malloc(sizeof *e);
    e->handle = CreateEventA(NULL, FALSE, FALSE, NULL);     /* auto-reset */
    return e;
}

void os_event_free(os_event *e) { CloseHandle(e->handle); free(e); }
void os_event_set(os_event *e) { SetEvent(e->handle); }

int os_event_wait(os_event *e, uint64_t nanos)
{
    DWORD ms = nanos == OS_FOREVER ? INFINITE : (DWORD)((nanos + 999999) / 1000000);
    return WaitForSingleObject(e->handle, ms) == WAIT_OBJECT_0;
}

#else

#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <time.h>

void *os_reserve(size_t reserve, size_t commit)
{
    void *base = mmap(NULL, reserve, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (base == MAP_FAILED) return NULL;
    if (commit && mprotect(base, commit, PROT_READ | PROT_WRITE)) {
        munmap(base, reserve);
        return NULL;
    }
    return base;
}

int os_commit(void *base, size_t bytes) { return mprotect(base, bytes, PROT_READ | PROT_WRITE); }
void os_release(void *base, size_t reserve) { if (base) munmap(base, reserve); }

void *os_lib_open(const char *path) { return dlopen(path, RTLD_NOW | RTLD_LOCAL); }
void *os_lib_sym(void *lib, const char *name) { return dlsym(lib, name); }
void  os_lib_close(void *lib) { if (lib) dlclose(lib); }

struct os_thread { pthread_t handle; void (*fn)(void *); void *arg; };

static void *thread_main(void *p)
{
    os_thread *t = p;
    t->fn(t->arg);
    return NULL;
}

os_thread *os_thread_start(void (*fn)(void *), void *arg)
{
    os_thread *t = malloc(sizeof *t);
    t->fn = fn;
    t->arg = arg;
    pthread_create(&t->handle, NULL, thread_main, t);
    return t;
}

void os_thread_join(os_thread *t)
{
    pthread_join(t->handle, NULL);
    free(t);
}

struct os_event { pthread_mutex_t lock; pthread_cond_t cond; int set; };

os_event *os_event_new(void)
{
    os_event *e = malloc(sizeof *e);
    pthread_mutex_init(&e->lock, NULL);
    pthread_cond_init(&e->cond, NULL);
    e->set = 0;
    return e;
}

void os_event_free(os_event *e)
{
    pthread_cond_destroy(&e->cond);
    pthread_mutex_destroy(&e->lock);
    free(e);
}

void os_event_set(os_event *e)
{
    pthread_mutex_lock(&e->lock);
    e->set = 1;
    pthread_cond_signal(&e->cond);
    pthread_mutex_unlock(&e->lock);
}

int os_event_wait(os_event *e, uint64_t nanos)
{
    struct timespec until;
    int got;
    clock_gettime(CLOCK_REALTIME, &until);
    until.tv_sec += (time_t)(nanos / 1000000000u);
    until.tv_nsec += (long)(nanos % 1000000000u);
    if (until.tv_nsec >= 1000000000L) { until.tv_sec ++; until.tv_nsec -= 1000000000L; }

    pthread_mutex_lock(&e->lock);
    while (!e->set) {
        if (nanos == OS_FOREVER) pthread_cond_wait(&e->cond, &e->lock);
        else if (pthread_cond_timedwait(&e->cond, &e->lock, &until) == ETIMEDOUT) break;
    }
    got = e->set;
    e->set = 0;
    pthread_mutex_unlock(&e->lock);
    return got;
}

#endif

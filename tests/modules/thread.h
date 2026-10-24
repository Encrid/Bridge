/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_THREAD_H
    #define TEST_THREAD_H

    #ifdef _WIN32
        #define WIN32_LEAN_AND_MEAN
        #include <windows.h>
    #else
        #include <pthread.h>
    #endif

typedef void (*thread_work_t)(void);

    #ifdef _WIN32

static inline DWORD WINAPI run_work(LPVOID work)
{
    (*(thread_work_t *)work)();
    return (0);
}

static inline void run_on_another_thread(thread_work_t work)
{
    HANDLE t = CreateThread(nullptr, 0, run_work, &work, 0, nullptr);

    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
}

    #else

static inline void *run_work(void *work)
{
    (*(thread_work_t *)work)();
    return (nullptr);
}

static inline void run_on_another_thread(thread_work_t work)
{
    pthread_t t;

    pthread_create(&t, nullptr, run_work, &work);
    pthread_join(t, nullptr);
}

    #endif

#endif

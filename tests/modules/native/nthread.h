/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nthread.h -- a function run on a thread of its own
** and waited for, which is spelled differently on Windows.
*/

#ifndef TEST_NTHREAD_H
    #define TEST_NTHREAD_H

    #ifdef _WIN32
        #define WIN32_LEAN_AND_MEAN
        #include <windows.h>
    #else
        #include <pthread.h>
    #endif

typedef void (*thread_work_t)(void);

    #ifdef _WIN32

/*
** run_work
**
** What the new thread begins in: it calls the function `work` points at.
*/
static inline DWORD WINAPI run_work(LPVOID work)
{
    (*(thread_work_t *)work)();
    return (0);
}

/*
** run_on_another_thread
**
** Starts a thread that calls `work`, and answers once that thread has
** ended.
*/
static inline void run_on_another_thread(thread_work_t work)
{
    HANDLE t = CreateThread(nullptr, 0, run_work, &work, 0, nullptr);

    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
}

    #else

/*
** run_work
**
** What the new thread begins in: it calls the function `work` points at.
*/
static inline void *run_work(void *work)
{
    (*(thread_work_t *)work)();
    return (nullptr);
}

/*
** run_on_another_thread
**
** Starts a thread that calls `work`, and answers once that thread has
** ended.
*/
static inline void run_on_another_thread(thread_work_t work)
{
    pthread_t t;

    pthread_create(&t, nullptr, run_work, &work);
    pthread_join(t, nullptr);
}

    #endif

#endif

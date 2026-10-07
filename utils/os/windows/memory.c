/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** utils/os/windows/memory.c -- address space on Windows: reserved, committed
** and given back.
*/

#include "os_types.h"

/*
** os_reserve
**
** Reserves `reserve` bytes and commits the first `commit` of them. nullptr on
** failure, with nothing left reserved.
*/
void *os_reserve(size_t reserve, size_t commit)
{
    void *base = VirtualAlloc(nullptr, reserve, MEM_RESERVE, PAGE_NOACCESS);

    if (!base || !commit)
        return (base);
    if (!VirtualAlloc(base, commit, MEM_COMMIT, PAGE_READWRITE)) {
        VirtualFree(base, 0, MEM_RELEASE);
        return (nullptr);
    }
    return (base);
}

/*
** os_commit
**
** Commits the first `bytes` of a reservation. 0 on success.
*/
int os_commit(void *base, size_t bytes)
{
    if (!VirtualAlloc(base, bytes, MEM_COMMIT, PAGE_READWRITE))
        return (-1);
    return (0);
}

/*
** os_release
**
** Gives a reservation back. Windows frees all of one by its base alone.
*/
void os_release(void *base, [[maybe_unused]] size_t reserve)
{
    if (base)
        VirtualFree(base, 0, MEM_RELEASE);
}

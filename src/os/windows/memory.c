/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "os_types.h"

void *os_reserve(size_t reserve, size_t commit)
{
    void *base = VirtualAlloc(nullptr, reserve, MEM_RESERVE, PAGE_NOACCESS);

    if (!base)
        return (nullptr);
    if (!VirtualAlloc(base, commit, MEM_COMMIT, PAGE_READWRITE)) {
        VirtualFree(base, 0, MEM_RELEASE);
        return (nullptr);
    }
    return (base);
}

int os_commit(void *base, size_t bytes)
{
    return (!VirtualAlloc(base, bytes, MEM_COMMIT, PAGE_READWRITE));
}

void os_release(void *base, [[maybe_unused]] size_t reserve)
{
    VirtualFree(base, 0, MEM_RELEASE);
}

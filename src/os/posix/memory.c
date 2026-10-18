/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <sys/mman.h>

#include "os_types.h"

void *os_reserve(size_t reserve, size_t commit)
{
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
    void *base = mmap(nullptr, reserve, PROT_NONE, flags, -1, 0);

    if (base == MAP_FAILED)
        return (nullptr);
    if (mprotect(base, commit, PROT_READ | PROT_WRITE)) {
        munmap(base, reserve);
        return (nullptr);
    }
    return (base);
}

int os_commit(void *base, size_t bytes)
{
    return (mprotect(base, bytes, PROT_READ | PROT_WRITE));
}

void os_release(void *base, size_t reserve)
{
    munmap(base, reserve);
}

/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** utils/os/posix/memory.c -- address space on Linux and macOS: mapped with no
** access, opened to reading and writing, and unmapped.
*/

#include <sys/mman.h>

#include "os_types.h"

/*
** os_reserve
**
** Maps `reserve` bytes nothing may touch, and opens the first `commit` of
** them. nullptr on failure, with nothing left mapped.
*/
void *os_reserve(size_t reserve, size_t commit)
{
    int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
    void *base = mmap(nullptr, reserve, PROT_NONE, flags, -1, 0);

    if (base == MAP_FAILED)
        return (nullptr);
    if (commit && mprotect(base, commit, PROT_READ | PROT_WRITE)) {
        munmap(base, reserve);
        return (nullptr);
    }
    return (base);
}

/*
** os_commit
**
** Opens the first `bytes` of a reservation to reading and writing. 0 on
** success.
*/
int os_commit(void *base, size_t bytes)
{
    return (mprotect(base, bytes, PROT_READ | PROT_WRITE));
}

/*
** os_release
**
** Unmaps a reservation. A null `base` is nothing to unmap.
*/
void os_release(void *base, size_t reserve)
{
    if (base)
        munmap(base, reserve);
}

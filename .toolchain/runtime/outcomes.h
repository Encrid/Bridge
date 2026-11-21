/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef C23_OUTCOMES_H
    #define C23_OUTCOMES_H

    #include <fcntl.h>
    #include <stddef.h>
    #include <stdlib.h>

    #ifdef _WIN32
        #include <io.h>

void *c23_outcomes_create_mapping(void *file, void *security,
    unsigned long protect, unsigned long size_high, unsigned long size_low,
    const char *name) __asm__("CreateFileMappingA");
void *c23_outcomes_map_view(void *mapping, unsigned long access,
    unsigned long offset_high, unsigned long offset_low, size_t size)
    __asm__("MapViewOfFile");
    #else
        #include <sys/mman.h>
        #include <unistd.h>
    #endif

typedef struct {
    _Atomic unsigned char *all;
    size_t size;
} c23_outcomes_file_t;

    #ifdef _WIN32

static inline _Atomic unsigned char *c23_outcomes_map(const char *path,
    size_t *size)
{
    int fd = _open(path, _O_RDWR | _O_BINARY);
    long long length = fd < 0 ? 0 : _filelengthi64(fd);
    void *mapping = nullptr;

    if (length > 0)
        mapping = c23_outcomes_create_mapping((void *)_get_osfhandle(fd),
            nullptr, 4ul, 0ul, 0ul, nullptr);
    if (fd >= 0)
        _close(fd);
    *size = length > 0 ? (size_t)length : 0;
    if (!mapping)
        return (nullptr);
    return (c23_outcomes_map_view(mapping, 2ul, 0ul, 0ul, 0));
}

    #else

static inline _Atomic unsigned char *c23_outcomes_map(const char *path,
    size_t *size)
{
    int fd = open(path, O_RDWR);
    off_t length = fd < 0 ? 0 : lseek(fd, 0, SEEK_END);
    void *all = MAP_FAILED;

    if (length > 0)
        all = mmap(nullptr, (size_t)length, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, 0);
    if (fd >= 0)
        close(fd);
    *size = length > 0 ? (size_t)length : 0;
    return (all == MAP_FAILED ? nullptr : all);
}

    #endif

static inline const c23_outcomes_file_t *c23_outcomes_get_file(void)
{
    static c23_outcomes_file_t file;
    static bool tried;
    const char *path = getenv("C23_BRANCHES");

    if (tried || !path)
        return (&file);
    tried = true;
    file.all = c23_outcomes_map(path, &file.size);
    if (!file.all)
        file.size = 0;
    return (&file);
}

static inline _Atomic unsigned char *c23_outcomes_keep(
    const unsigned char *bytes, size_t start, size_t count)
{
    const c23_outcomes_file_t *file = c23_outcomes_get_file();

    if (!file->all || start > file->size || count > file->size - start)
        return (nullptr);
    for (size_t i = 0; i < count; i++)
        if (bytes[i])
            file->all[start + i] = 1;
    return (file->all + start);
}

#endif

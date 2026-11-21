/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef C23_COUNTS_H
    #define C23_COUNTS_H

    #include <fcntl.h>
    #include <stddef.h>
    #include <stdlib.h>
    #include <string.h>

    #ifdef _WIN32
        #include <errno.h>
        #include <io.h>
        #include <share.h>
        #include <sys/stat.h>
    #else
        #include <unistd.h>
    #endif

    #define C23_COUNTS_ROOM(length, slots) ((length) + 2 + 22 * (slots))

static inline size_t c23_counts_put_number(char *line, size_t at,
    long long n)
{
    char digits[24];
    unsigned long long left = n < 0 ? 0ull - (unsigned long long)n
        : (unsigned long long)n;
    size_t count = 0;

    if (n < 0)
        line[at++] = '-';
    do {
        digits[count++] = (char)('0' + left % 10u);
        left /= 10u;
    } while (left);
    while (count)
        line[at++] = digits[--count];
    return (at);
}

    #ifdef _WIN32

static inline bool c23_counts_append(const char *line, size_t length)
{
    const char *path = getenv("C23_LEAKS");
    int fd = -1;
    bool written;

    if (!path)
        return (false);
    do
        fd = _sopen(path, _O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY,
            _SH_DENYRW, _S_IREAD | _S_IWRITE);
    while (fd < 0 && errno == EACCES);
    if (fd < 0)
        return (false);
    written = _write(fd, line, (unsigned)length) == (int)length;
    _close(fd);
    return (written);
}

    #else

static inline bool c23_counts_append(const char *line, size_t length)
{
    const char *path = getenv("C23_LEAKS");
    int fd = path ? open(path, O_WRONLY | O_CREAT | O_APPEND, 0644) : -1;
    bool written;

    if (fd < 0)
        return (false);
    written = write(fd, line, length) == (ssize_t)length;
    close(fd);
    return (written);
}

    #endif

static inline bool c23_counts_keep(char *line, const char *path,
    const long long *counts, size_t slots)
{
    size_t at = strlen(path);

    memcpy(line, path, at);
    for (size_t i = 0; i < slots; i++) {
        line[at++] = ' ';
        at = c23_counts_put_number(line, at, counts[i]);
    }
    line[at++] = '\n';
    return (c23_counts_append(line, at));
}

#endif

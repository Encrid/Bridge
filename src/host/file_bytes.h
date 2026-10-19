/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_HOST_FILE_BYTES_H
    #define BRIDGE_HOST_FILE_BYTES_H

    #include <stddef.h>
    #include <stdint.h>

[[nodiscard]] char *read_whole(const char *path, size_t *len);

uint64_t hash_bytes(const char *bytes, size_t len);

#endif

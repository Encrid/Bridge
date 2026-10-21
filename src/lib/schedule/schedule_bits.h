/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_SCHEDULE_BITS_H
    #define BRIDGE_SRC_SCHEDULE_BITS_H

    #include <stddef.h>
    #include <stdint.h>

    #include "bits.h"

static inline size_t schedule_get_matrix_words(uint32_t n)
{
    return ((size_t)n * bits_words(n) + 1);
}

#endif

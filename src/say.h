/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_SAY_H
    #define BRIDGE_SRC_SAY_H

    #include "inside.h"

typedef struct {
    const module_t *m;
    const bridge_system_t *s;
    uint8_t level;
} say_t;

[[gnu::format(printf, 2, 3)]] void say_error(bridge_t *b, const char *format,
    ...);

[[gnu::format(printf, 3, 4)]] void say_about(bridge_t *b, const say_t *about,
    const char *format, ...);

[[nodiscard]] char *copy_text(const char *s, size_t n);

[[nodiscard]] int bridge_lacks(bridge_t *b, const void *got, const char *what);

#endif

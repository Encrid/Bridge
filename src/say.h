/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/say.h -- a record of Bridge-ASM700's own, and what every file of the
** library says when there is no memory.
*/

#ifndef BRIDGE_SRC_SAY_H
    #define BRIDGE_SRC_SAY_H

    #include "inside.h"

/*
** Whose a record is about, when it is anybody's, and how bad it is.
*/
typedef struct {
    const module_t *m;
    const bridge_system_t *s;
    uint8_t level;
} say_t;

/*
** say_error
**
** Hands the sink an error of Bridge-ASM700's own, about no module, written
** as printf would.
*/
[[gnu::format(printf, 2, 3)]] void say_error(bridge_t *b, const char *format,
    ...);

/*
** say_about
**
** Hands the sink a record of Bridge-ASM700's own about a module or one of
** its systems, at the level `about` states.
*/
[[gnu::format(printf, 3, 4)]] void say_about(bridge_t *b, const say_t *about,
    const char *format, ...);

/*
** copy_text
**
** Answers a copy of the first `n` bytes of a text, with a zero after them.
** nullptr if there was not the memory for it: whoever asked says so.
*/
[[nodiscard]] char *copy_text(const char *s, size_t n);

/*
** bridge_lacks
**
** Answers 1, with a record that says what for, if `got` is nullptr: there
** was no memory for `what`.
**
** failure.md, *Out of memory*
*/
[[nodiscard]] int bridge_lacks(bridge_t *b, const void *got, const char *what);

#endif

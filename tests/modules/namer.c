/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/namer.c -- keeps a string in a component: the bytes
** in the persistent arena, the component holding where they are and how
** many.
**
** Built a second way, READER: it declares the same component and says what
** it finds there, and keeps nothing of its own. That is for a string that
** outlives the module that wrote it.
**
** strings.md, *Variable length*
*/

#include "test_offsets.h"
#include "namer.h"
#include "offsets_idle.h"

[[maybe_unused]] static const char HELLO[] = "kept in the arena";

static uint32_t name;
static uint32_t saying;

/*
** say
**
** Says the string the first row holds, up to 32 bytes of it, and nothing
** when there is no row.
*/
BRIDGE_EXPORT(say) int say(void)
{
    char text[32];
    uint32_t span;
    uint32_t at;
    uint32_t len;

    if (!bridge_count_spans(saying))
        return (0);
    span = bridge_get_span(saying, 0, 1);
    at = BRIDGE_ROW(name_t, bridge_get_base(span, 0), 0,
        bridge_count_rows(span));
    len = BRIDGE_GET(name_t, text.len, at);
    if (len > sizeof(text))
        len = sizeof(text);
    acc_from_shared(text, BRIDGE_GET(name_t, text.at, at), len);
    bridge_log(BRIDGE_LOG_INFO, &(bridge_line_t){ text, len, 0, 0 });
    return (0);
}

/*
** init
**
** Declares the component and the system. The build that is not READER then
** takes the string's bytes from the persistent arena, copies the string
** there, and spawns one entity whose component says where it is.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    [[maybe_unused]] name_t n = {{ 0, sizeof(HELLO) - 1 }};

    name = BRIDGE_COMPONENT("game:Name", name_t, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = name;
    saying = BRIDGE_ADD_SYSTEM("say", "update", terms, 1, nullptr, 0);
#ifndef READER
    n.text.at = bridge_alloc(n.text.len);
    acc_to_shared(n.text.at, HELLO, n.text.len);
    BRIDGE_ATTACH(bridge_spawn(), name, n);
#endif
    return (0);
}

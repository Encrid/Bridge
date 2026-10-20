/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "namer.h"
#include "wasm_idle.h"

[[maybe_unused]] static const char HELLO[] = "kept in the arena";

static uint32_t name;
static uint32_t saying;

BRIDGE_EXPORT(say) int say(void)
{
    char text[32];
    uint32_t span;
    uint32_t table;
    uint32_t len;

    if (!bridge_count_spans(saying))
        return (0);
    span = bridge_get_span(saying, 0);
    table = bridge_span_table(span);
    len = BRIDGE_GET(name_t, text.len, name, table, 0);
    if (len > sizeof(text))
        len = sizeof(text);
    acc_from_shared(text, BRIDGE_GET(name_t, text.at, name, table, 0), len);
    bridge_log(BRIDGE_LOG_INFO, &(bridge_line_t){ text, len, 0, 0 });
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    [[maybe_unused]] name_t n = {{ 0, sizeof(HELLO) - 1 }};

    name = BRIDGE_COMPONENT("game:Name", name_t, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = name;
    saying = BRIDGE_ADD_SYSTEM("say", "update", terms, 1, nullptr, 0, nullptr, 0);
#ifndef READER
    n.text.at = bridge_alloc(n.text.len);
    acc_to_shared(n.text.at, HELLO, n.text.len);
    BRIDGE_ATTACH(bridge_spawn(), name, n);
#endif
    return (0);
}

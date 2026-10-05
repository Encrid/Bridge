/* Keeps a string in a component: the bytes in the persistent arena, the component holding
 * where they are and how many. docs/bridge/memory/strings.md.
 *
 * Built a second way, READER: it declares the same component and says what it finds there, and
 * keeps nothing of its own. For a string that outlives the module that wrote it. */
#include "bridge-wasm.h"

BRIDGE_SCHEMA_STRUCT(Name, { bridge_text text; });

static uint32_t name, saying;

BRIDGE_EXPORT(say) int say()
{
    uint32_t span;
    if (!bridge_spans(saying)) return 0;
    span = bridge_span(saying, 0, 1);
    uint32_t at = BRIDGE_ROW(Name, bridge_base(span, 0), 0, bridge_rows(span));
    uint32_t len = BRIDGE_GET(Name, text.len, at);
    char text[32];
    if (len > sizeof text) len = sizeof text;
    acc_from_shared(text, BRIDGE_GET(Name, text.at, at), len);
    bridge_log(BRIDGE_LOG_INFO, text, len, 0, 0);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    [[maybe_unused]] static const char hello[] = "kept in the arena";
    bridge_term terms[1] = {{ 0, BRIDGE_IN, BRIDGE_AND, 0 }};
    [[maybe_unused]] Name n = {{ 0, sizeof hello - 1 }};
    name = BRIDGE_COMPONENT("game:Name", Name, BRIDGE_DELETE);
    terms[0].slot = name;
    saying = BRIDGE_SYSTEM("say", "update", terms, 1);
#ifndef READER
    n.text.at = bridge_alloc(n.text.len);
    acc_to_shared(n.text.at, hello, n.text.len);
    BRIDGE_ATTACH(bridge_spawn(), name, n);
#endif
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }

/* Keeps a string in a component: the bytes in the persistent arena, the component holding
 * where they are and how many. docs/bridge/memory/strings.md. */
#include "bridge-wasm.h"

ECS_STRUCT(Name, { uint32_t at; uint32_t len; });

static uint32_t name, saying;

BRIDGE_EXPORT(say) int say(void)
{
    uint32_t span = bridge_span(saying, 0, 1);
    uint32_t at = BRIDGE_ROW(Name, bridge_base(span, 0), 0, bridge_rows(span));
    uint32_t len = BRIDGE_GET(Name, len, at);
    char text[32];
    if (len > sizeof text) len = sizeof text;
    acc_from_shared(text, BRIDGE_GET(Name, at, at), len);
    bridge_log(BRIDGE_LOG_INFO, text, len, 0, 0);
    return 0;
}

BRIDGE_EXPORT(init) int init(void)
{
    static const char hello[] = "kept in the arena";
    bridge_term terms[1] = {{ 0, BRIDGE_IN, BRIDGE_AND, 0 }};
    Name n = { 0, sizeof hello - 1 };
    name = BRIDGE_COMPONENT("game:Name", Name, BRIDGE_DELETE);
    terms[0].slot = name;
    saying = BRIDGE_SYSTEM("say", "update", terms, 1);
    n.at = bridge_alloc(n.len);
    acc_to_shared(n.at, hello, n.len);
    BRIDGE_ATTACH(bridge_spawn(), name, n);
    return 0;
}

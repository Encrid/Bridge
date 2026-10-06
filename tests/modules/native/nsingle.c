/* A singleton: one value for the deployment, on no entity. docs/bridge/storage.md, *A
 * singleton*.
 *
 *     attach_a         it is given a value, with the id 0: heard, as an entity's is
 *     value_a          a value for it when it has one: heard as an attach
 *     work             a field of it written where it is: no call, and nothing is heard
 *     detach_b         it is taken: it has no value
 *     look             what a system reads of it: whether it has a value, and the value
 *     one              it read a million times, found again each time, for a measurement
 *
 * BAD is a build the bridge refuses something of: 1 a system that selects by it, at load; 2 an
 * attach of it with the id of an entity, when that system runs; 3 an attach of a component
 * that is no singleton with the id 0. */
#include "bridge_native.h"

#define G_FIELDS { uint32_t gravity; uint32_t wind; }
BRIDGE_SCHEMA_STRUCT_TYPE(G, G_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(G, G_FIELDS);
#define A_FIELDS { uint32_t v; }
BRIDGE_SCHEMA_STRUCT_TYPE(A, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(A, A_FIELDS);

static uint32_t g, a, on_g;

static void say(const bridge_api_t *api, const char *text, uint32_t x, uint32_t y)
{
    bridge_native_log(api, BRIDGE_LOG_INFO, &(bridge_line_t){ text, (uint32_t)strlen(text), x, y });
}

BRIDGE_NATIVE(attach_a)
{
    G value = { 10, 1 };
#if defined BAD && BAD == 2
    BRIDGE_NATIVE_ATTACH(api, api->spawn(api), g, value);
#elif defined BAD && BAD == 3
    A other = { 1 };
    BRIDGE_NATIVE_ATTACH(api, 0, a, other);
#endif
    BRIDGE_NATIVE_ATTACH(api, 0, g, value);
    return 0;
}

BRIDGE_NATIVE(value_a)
{
    G value = { 20, 2 };
    BRIDGE_NATIVE_ATTACH(api, 0, g, value);
    return 0;
}

BRIDGE_NATIVE(work)
{
    G *value = bridge_native_get_single(api, g);
    if (value) value->wind = 77;
    return 0;
}

BRIDGE_NATIVE(detach_b)
{
    api->detach(api, 0, g);
    return 0;
}

BRIDGE_NATIVE(look)
{
    const G *value = bridge_native_get_single(api, g);
    if (value) say(api, "it has a value: gravity, wind", value->gravity, value->wind);
    else say(api, "it has no value", 0, 0);
    return 0;
}

/* which singleton `one` reads, behind a load the compiler cannot see through */
static volatile uint32_t which;

BRIDGE_NATIVE(one)
{
    uint32_t total = 0;
    for (uint32_t i = 0; i < 1'000'000; i ++) {
        const G *value = bridge_native_get_single(api, which);
        if (value) total += value->gravity;
    }
    say(api, "read a million times: the sum", total, 0);
    return 0;
}

BRIDGE_NATIVE(heard_g)
{
    const bridge_list_t *list = bridge_native_get_list(api, on_g);
    auto trigger = (const bridge_trigger_t *)(api->private_base + list->at);
    for (uint32_t i = 0; i < list->count; i ++) {
        say(api, "heard of it: the id, events", (uint32_t)trigger[i].id, trigger[i].events);
    }
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};
    uint32_t both[2];
    g = both[0] = BRIDGE_NATIVE_COMPONENT(api, "st:G", G, BRIDGE_DELETE, BRIDGE_SINGLETON);
    a = both[1] = BRIDGE_NATIVE_COMPONENT(api, "st:A", A, BRIDGE_DELETE, BRIDGE_TABLE);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_a", "attach_a", &none, 0, both, 2);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "value_a", "value_a", &none, 0, &g, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0, &g, 1);
    one[0].slot = g;
    BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", one, 1, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "one", "one", one, 1, nullptr, 0);
    which = g;
    one[0].access = BRIDGE_WRITE;
    BRIDGE_NATIVE_ADD_SYSTEM(api, "work", "work", one, 1, nullptr, 0);
    one[0].access = BRIDGE_READ;
    one[0].oper = BRIDGE_AND;
    on_g = BRIDGE_NATIVE_OBSERVER(api, "heard_g",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr, 0);
#if defined BAD && BAD == 1
    BRIDGE_NATIVE_ADD_SYSTEM(api, "by", "never", one, 1, nullptr, 0);
#endif
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }

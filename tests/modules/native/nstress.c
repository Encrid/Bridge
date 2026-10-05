/* Storage under load, with its own account of what should be there.
 *
 * Each pass it asks for twenty thousand changes chosen by a generator it seeds itself: an
 * entity made with a few components, one destroyed, a component attached, set or detached.
 * It keeps what each of its entities ought to carry and with what value, and at the next pass
 * looks: at every entity through the index and column_base, and at every component through its
 * spans. What it says is how many disagreed, which must be none, and a digest of its own
 * account, which is the same whatever keeps the rows and on however many threads.
 *
 * Built several times, each under its own namespace, so that they run at once.
 * docs/plan/PHASE10.md. */
#include "bridge-native.h"

BRIDGE_SCHEMA_STRUCT(V, { uint32_t v; });

constexpr uint32_t KINDS = 8, SLOTS = 50'000, CHANGES = 20'000;

static const bridge_api *api;
static uint32_t kinds[KINDS], seeing[KINDS];
static uint64_t ids[SLOTS];
static uint8_t  has[SLOTS];             /* which kinds it ought to carry: a bit each */
static uint32_t values[SLOTS][KINDS];
static uint32_t state = SEED, passes, wrong;

BRIDGE_NATIVE_BIND(api)

static uint32_t next()
{
    state = state * 1'664'525u + 1'013'904'223u;
    return state >> 8;
}

static void set(uint32_t slot, uint32_t kind)
{
    V value = { next() };
    api->attach(ids[slot], kinds[kind], bridge_native_put(api, &value, sizeof value), sizeof value);
    has[slot] |= (uint8_t)(1u << kind);
    values[slot][kind] = value.v;
}

BRIDGE_NATIVE(work)
{
    for (uint32_t n = 0; n < CHANGES; n ++) {
        uint32_t slot = next() % SLOTS, what = next() % 100, kind = next() % KINDS;
        if (!ids[slot]) {
            ids[slot] = api->spawn();
            has[slot] = 0;
            for (uint32_t k = 1 + next() % 3; k --;) set(slot, next() % KINDS);
        } else if (what < 15) {
            api->destroy(ids[slot]);
            ids[slot] = 0;
        } else if (what < 70) {
            set(slot, kind);                /* an attach, or a set where it has it */
        } else {
            api->detach(ids[slot], kinds[kind]);
            has[slot] &= (uint8_t)~(1u << kind);
        }
    }
    return 0;
}

/* every entity, by its id: where the index says it is, and what column_base says is there */
static void by_reference()
{
    const bridge_header *h = bridge_native_header(api);
    auto index = (const bridge_entity *)(api->shared_base + h->entity_index);
    auto column_base = (const uint32_t *)(api->private_base + h->column_base);
    uint32_t live = 0, digest = 0;
    for (uint32_t slot = 0; slot < SLOTS; slot ++) {
        if (!ids[slot]) continue;
        const bridge_entity *record = &index[(uint32_t)ids[slot]];
        live ++;
        digest = digest * 31 + has[slot];
        if (record->generation != (uint32_t)(ids[slot] >> 32)) {
            wrong ++;
            continue;
        }
        for (uint32_t k = 0; k < KINDS; k ++) {
            uint32_t base = column_base[record->table * h->terms + kinds[k]];
            if (!base != !(has[slot] >> k & 1)) wrong ++;
            else if (base) {
                wrong += ((const V *)(api->shared_base + base))[record->row].v != values[slot][k];
                digest = digest * 31 + values[slot][k];
            }
        }
    }
    passes ++;
    if (passes % 25 == 0) {
        bridge_native_log(api, 0, "live, digest", 12, live, digest);
        bridge_native_log(api, 0, "disagreements so far", 20, wrong, 0);
    }
}

/* and every component, by its spans: as many rows as carry it, holding what they should */
static int by_spans(uint32_t kind)
{
    const bridge_query *q = bridge_native_query(api, seeing[kind]);
    uint32_t rows = 0, sum = 0, want_rows = 0, want_sum = 0;
    for (uint32_t s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 1);
        auto column = (const V *)(api->shared_base + span[2]);
        rows += span[0];
        for (uint32_t r = 0; r < span[0]; r ++) sum += column[r].v;
    }
    for (uint32_t slot = 0; slot < SLOTS; slot ++) {
        if (!ids[slot] || !(has[slot] >> kind & 1)) continue;
        want_rows ++;
        want_sum += values[slot][kind];
    }
    wrong += rows != want_rows || sum != want_sum;
    if (!kind) by_reference();
    return 0;
}

BRIDGE_NATIVE(see0) { return by_spans(0); }
BRIDGE_NATIVE(see1) { return by_spans(1); }
BRIDGE_NATIVE(see2) { return by_spans(2); }
BRIDGE_NATIVE(see3) { return by_spans(3); }
BRIDGE_NATIVE(see4) { return by_spans(4); }
BRIDGE_NATIVE(see5) { return by_spans(5); }
BRIDGE_NATIVE(see6) { return by_spans(6); }
BRIDGE_NATIVE(see7) { return by_spans(7); }

BRIDGE_NATIVE(init)
{
    static const char *const names[KINDS] = {
        SPACE ":K0", SPACE ":K1", SPACE ":K2", SPACE ":K3",
        SPACE ":K4", SPACE ":K5", SPACE ":K6", SPACE ":K7"
    };
    static const char *const systems[KINDS] = {
        "see0", "see1", "see2", "see3", "see4", "see5", "see6", "see7"
    };
    int none = 0;
    for (uint32_t k = 0; k < KINDS; k ++) {
        bridge_term term[1] = {{ 0, BRIDGE_IN, BRIDGE_AND, 0 }};
        kinds[k] = api->declare(bridge_native_put(api, names[k], (uint32_t)strlen(names[k])),
            (uint32_t)strlen(names[k]), BRIDGE_STRUCT, sizeof(V), alignof(V),
            bridge_native_put(api, V_desc, sizeof V_desc - 1), sizeof V_desc - 1, BRIDGE_DELETE);
        term[0].slot = kinds[k];
        seeing[k] = api->system(bridge_native_put(api, systems[k], 4), 4,
            bridge_native_put(api, "check", 5), 5,
            bridge_native_put(api, term, sizeof term), 1);
    }
    BRIDGE_NATIVE_SYSTEM(api, "work", "work", &none, 0);
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }

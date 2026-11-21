/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "storage.h"
#include "storage_test.h"

storage_type_t *storage_test_find_type(const bridge_world_t *w,
    const char *name)
{
    uint32_t i = 0;

    while (i < w->type_count && strcmp(w->types[i]->name, name))
        i++;
    return (i < w->type_count ? w->types[i]->type : nullptr);
}

void storage_test_count_value(void *ctx, const uint8_t *value)
{
    uint32_t *count = ctx;

    (void)value;
    (*count)++;
}

static void count_changed(void *ctx, const storage_type_t *type)
{
    uint32_t *count = ctx;

    (void)type;
    (*count)++;
}

static void scan_twice(storage_test_t *t, const char *phase)
{
    storage_t *r = t->w->storage;
    uint32_t changed[2] = {};

    storage_validate_baseline(r);
    storage_validate_scan(r, count_changed, &changed[0]);
    storage_test_run(t, phase);
    storage_validate_scan(r, count_changed, &changed[1]);
    printf("columns changed with no write: %u; after %s: %u\n", changed[0],
        phase, changed[1]);
}

enum storage_test_end storage_test_call_on_rows(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(TOGGLE_PATH, 16, 1 << 20, 4096);
    storage_t *r = t->w->storage;
    storage_type_t *plain;
    storage_type_t *toggle;
    uint32_t read = 0;

    if (storage_test_try_load(t, &a))
        return (STORAGE_TEST_DONE);
    storage_test_run(t, "spawn");
    storage_test_run(t, "flicker");
    plain = storage_test_find_type(t->w, "st:A");
    toggle = storage_test_find_type(t->w, "st:T");
    storage_visit_values(r, plain, storage_test_count_value, &read);
    printf("values of st:A read: %u; rows with st:T off: %u; with st:A off: "
        "%u\n", read, storage_count_off(r, toggle), storage_count_off(r, plain));
    scan_twice(t, "value_a");
    return (STORAGE_TEST_DONE);
}

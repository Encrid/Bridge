/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/abi_relay.c -- a type that has another component layout at a reload:
** how its values are carried over, or why they cannot be.
*/

#include <stdlib.h>
#include <string.h>

#include "abi_declare.h"

static const char *const KIND_NAMES[] = {
    "",
    "a struct",
    "an enum",
    "a bitmask",
    "a tag"
};

/*
** check_constant
**
** Refuses a constant of an enum or a bitmask that is gone, or has another
** value: every value a row may hold must still mean what it meant.
**
** lifetime.md, *When a component layout changed*
*/
static int check_constant(module_t *m, const declared_type_t *was,
    const schema_t *is, const schema_constant_t *old)
{
    const schema_constant_t *new = nullptr;
    uint32_t k = 0;

    while (k < is->constant_count) {
        if (!strcmp(is->constants[k].name, old->name))
            new = &is->constants[k];
        k++;
    }
    if (!new)
        return (abi_refuse(m, "type '%s' has no constant '%s' here, which it "
            "had: a value kept of it would mean nothing", was->name,
            old->name));
    if (new->value == old->value)
        return (0);
    return (abi_refuse(m, "type '%s': its constant '%s' is %lld here and was "
        "%lld, and every value kept of it would mean something else",
        was->name, old->name, (long long)new->value, (long long)old->value));
}

/*
** check_constants
**
** Refuses an enum or a bitmask whose integer or whose constants changed. A
** constant added is nothing.
**
** lifetime.md, *When a component layout changed*
*/
static int check_constants(module_t *m, const declared_type_t *was,
    const schema_t *is)
{
    uint32_t i = 0;

    if (was->s.size != is->size)
        return (abi_refuse(m, "type '%s' is an integer of %u bytes here and "
            "was one of %u: every value kept of it would be read as another",
            was->name, is->size, was->s.size));
    while (i < was->s.constant_count) {
        if (check_constant(m, was, is, &was->s.constants[i]))
            return (1);
        i++;
    }
    return (0);
}

/*
** keep_runs
**
** Leaves on a type how its values are carried over, or refuses the relay
** for what it found: no memory, or a loss this reload does not accept.
**
** lifetime.md, *When a component layout changed*
** failure.md, *Out of memory*
*/
static int keep_runs(relaying_t *l, declared_type_t *was)
{
    if (l->short_of)
        return (abi_refuse(l->m, "out of memory: there was none to work out "
            "how '%s' is laid out again", was->name));
    if (l->lost_len)
        return (abi_refuse(l->m, "type '%s' has another component layout "
            "here, and %u rows would lose %s. A reload that accepts that "
            "states each among its losses", was->name, l->rows, l->lost));
    was->runs = l->runs ? l->runs : calloc(1, sizeof(*was->runs));
    was->run_count = l->run_count;
    l->runs = nullptr;
    if (was->runs)
        return (0);
    return (abi_refuse(l->m, "out of memory: there was none to work out how "
        "'%s' is laid out again", was->name));
}

/*
** relay_struct
**
** Works out how the values of a struct are carried over to what it becomes.
*/
static int relay_struct(module_t *m, declared_type_t *was, const schema_t *is)
{
    storage_t *storage = m->b->storage;
    relaying_t l = { .m = m };
    relay_at_t at = { &was->s, is, 0, 0, was->name };
    int refused;

    l.rows = storage_count_carriers(storage, was->type);
    refused = abi_relay_fields(&l, &at);
    if (!refused)
        refused = keep_runs(&l, was);
    free(l.runs);
    return (refused);
}

/*
** abi_relay
**
** Works out how the values of a type are carried over to what it becomes. A
** reload does not change what kind of type one is; a tag has no value to
** carry.
**
** lifetime.md, *When a component layout changed*
*/
int abi_relay(module_t *m, declared_type_t *was, enum bridge_kind kind,
    const schema_t *is)
{
    if (was->kind != kind)
        return (abi_refuse(m, "type '%s' is %s here and was %s: a reload does "
            "not change what kind of type one is", was->name,
            KIND_NAMES[kind], KIND_NAMES[was->kind]));
    if (kind == BRIDGE_TAG)
        return (0);
    if (kind != BRIDGE_STRUCT)
        return (check_constants(m, was, is));
    return (relay_struct(m, was, is));
}

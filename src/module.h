/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/module.h -- the runtime lifecycle: what a load, an unload and a
** reload share. All three are legal only between phases.
**
** lifecycle.md, *The runtime lifecycle*
** lifetime.md, *What unload leaves behind*
** failure.md, *When something fails*
*/

#ifndef BRIDGE_SRC_MODULE_H
    #define BRIDGE_SRC_MODULE_H

    #include "config.h"
    #include "inside.h"

/*
** Where a module's scratch arena begins in its private memory: after the
** header.
*/
constexpr uint32_t MODULE_SCRATCH_AT = 128;

/*
** An artifact being checked, before it is touched: everything a load and a
** reload both refuse from a config alone.
**
**     cfg, id       its config, and the identity it states
**     on            the backend it names
**     demands       what its config asks that backend to keep
*/
typedef struct {
    bridge_t *b;
    const bridge_artifact_t *a;
    const bridge_module_config_t *cfg;
    const char *id;
    open_backend_t *on;
    enum backend_capability demands;
} module_check_t;

/*
** Where a module's regions sit in its private memory: the header, then
** scratch, the log ring and the spans, each at a multiple of 8.
*/
typedef struct {
    uint32_t ring;
    uint32_t spans;
    uint32_t end;
} module_private_t;

/*
** What is looked for in the rows of a type: a string that lies in a
** module's share of the persistent arena. `rows` is how many hold one.
*/
typedef struct {
    const module_t *m;
    const declared_type_t *c;
    uint32_t rows;
} module_pointing_t;

/*
** One call of a lifecycle export: what is called, its name, and its
** timeout.
*/
typedef struct {
    backend_func_t *f;
    const char *name;
    int64_t timeout;
} module_call_t;

/*
** What a module is started from: its artifact, what its config demands of
** its backend, and the first call into it, `init` for a load and `reload`
** for a reload.
*/
typedef struct {
    const bridge_artifact_t *a;
    enum backend_capability demands;
    const char *entry;
} module_start_t;

/*
** module_refuse
**
** Says that a module is refused, and why, written as printf would. Answers
** 1.
*/
[[gnu::format(printf, 3, 4)]] int module_refuse(bridge_t *b,
    const char *identity, const char *format, ...);

/*
** module_fail
**
** Writes why a module's load or reload failed, as printf would. Answers 1.
*/
[[gnu::format(printf, 2, 3)]] int module_fail(backend_err_t *err,
    const char *format, ...);

/*
** module_check_set
**
** Refuses a key whose zero means nobody stated it. 1 if refused.
**
** config.md, *No unaccountable values*
*/
int module_check_set(const module_check_t *k, const char *key, int64_t value);

/*
** module_check_bound
**
** Refuses a negative number that is not `none`, which is BRIDGE_NO_LIMIT or
** BRIDGE_NONE: a key that has a "none" has one, and any other is nobody's.
**
** config.md, *No unaccountable values*
*/
int module_check_bound(const module_check_t *k, const char *key,
    int64_t value, int64_t none);

/*
** module_check_flag
**
** Refuses a key that is yes or no and is stated as neither.
**
** config.md, *No unaccountable values*
*/
int module_check_flag(const module_check_t *k, const char *key,
    enum bridge_flag value);

/*
** module_check_flags
**
** Refuses a config with a flag left unset.
*/
int module_check_flags(const module_check_t *k);

/*
** module_check_phases
**
** Refuses a config whose phase names are unset, mapped twice, or mapped to
** a phase the deployment did not declare.
**
** schedule.md, *A module's phase names are mapped by the deployment*
*/
int module_check_phases(const module_check_t *k);

/*
** module_check_stated
**
** Refuses a config with a policy, a limit or an amount left unset, or
** stated as a number that is nobody's.
*/
int module_check_stated(const module_check_t *k);

/*
** module_check_fit
**
** Refuses a config whose amounts do not fit: a log level there is not, more
** spawns than there are entities, regions that an offset of 32 bits cannot
** reach, a log ring that holds no record.
*/
int module_check_fit(const module_check_t *k);

/*
** module_check_backend
**
** Refuses a config that states what its backend cannot keep, or allows what
** its author says cannot be done.
**
** capabilities.md, *Capabilities, so the swap is honest*
*/
int module_check_backend(const module_check_t *k);

/*
** module_check
**
** Refuses everything a load and a reload both refuse before the artifact is
** touched, and finds the backend it names and what it demands of it. 1 with
** a record.
*/
[[nodiscard]] int module_check(module_check_t *k);

/*
** module_check_room
**
** Refuses a load of a module that is loaded already, that is one more than
** the budget of modules, or whose share does not fit in what is left of the
** persistent arena. 1 with a record.
*/
[[nodiscard]] int module_check_room(const module_check_t *k);

/*
** module_find
**
** Answers the loaded module of an identity, whatever its version, or
** nullptr. A system is named by its module without the version, so that is
** what tells modules apart.
**
** schedule.md, *Order within a phase is never implicit*
*/
module_t *module_find(bridge_t *b, const char *identity);

/*
** module_lay_private
**
** Works out where a module's regions sit in its private memory. 1 if what
** it stated cannot be laid out in 32 bits, which is what every offset into
** that memory is.
*/
int module_lay_private(const bridge_module_config_t *cfg, uint64_t granule,
    module_private_t *at);

/*
** module_take_private
**
** Reserves a module's private memory and hands it to its backend. 1 with
** `err`.
*/
[[nodiscard]] int module_take_private(module_t *m,
    const bridge_module_config_t *cfg, backend_err_t *err);

/*
** module_record
**
** Makes a module's record from its config: all of it, or none. nullptr with
** a record said.
**
** failure.md, *Out of memory*
*/
[[nodiscard]] module_t *module_record(bridge_t *b, const bridge_artifact_t *a,
    open_backend_t *o);

/*
** module_drop_systems
**
** Takes a module's systems and observers out of storage and frees them.
*/
void module_drop_systems(bridge_t *b, module_t *m);

/*
** module_release_types
**
** A module stops declaring what it declared, last declared first. A type
** others declare stays; the last to leave applies its own policy, or none
** if `by_policy` is false: nothing can carry what was never applied.
*/
void module_release_types(bridge_t *b, module_t *m, bool by_policy);

/*
** module_give_share
**
** Gives a module's share of the persistent arena back to it.
*/
void module_give_share(bridge_t *b, const module_t *m);

/*
** module_settle_share
**
** Gives a module's share of the persistent arena back at its unload, unless
** a row that is staying holds a string in it.
**
** strings.md, *It is given back when nothing points into it*
*/
void module_settle_share(bridge_t *b, module_t *m);

/*
** module_count_pointing
**
** Answers how many rows of a type hold a string in a module's share of the
** persistent arena.
*/
uint32_t module_count_pointing(const bridge_t *b, const module_t *m,
    const declared_type_t *c);

/*
** module_call
**
** Calls a lifecycle export once, under its timeout. 0 if it returned 0, and
** otherwise 1 with why in `err`.
*/
[[nodiscard]] int module_call(module_t *m, const module_call_t *call,
    backend_err_t *err);

/*
** module_make_instance
**
** Compiles a module's image, reserves its private memory, and instantiates
** it with the memories it imports and the ABI. 1 with `err`.
*/
[[nodiscard]] int module_make_instance(module_t *m, const module_start_t *st,
    backend_err_t *err);

/*
** module_start
**
** An image, an instance, a private memory, and the first call into it. 0 if
** the module registered and every export it named is there.
*/
[[nodiscard]] int module_start(module_t *m, const module_start_t *st,
    backend_err_t *err);

#endif

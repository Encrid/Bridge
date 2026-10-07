/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/inside.h -- Bridge's inside: what a deck, a module, a system
** and a declared type are. Nothing here crosses a seam.
*/

#ifndef BRIDGE_SRC_INSIDE_H
    #define BRIDGE_SRC_INSIDE_H

    #include "bridge.h"
    #include "bridge_abi.h"
    #include "backend.h"
    #include "os.h"
    #include "pool.h"
    #include "region.h"
    #include "schema.h"
    #include "storage.h"

typedef struct module module_t;
typedef struct declared_type declared_type_t;

/*
** A backend the deployment configured, opened. `shared` is the shared block,
** as this backend sees it. `reservation`, `guard` and `granule` are the
** address space it needs around a block it maps: a module's private block
** has this, and the shared block the largest any backend stated.
**
** backend.md, *Memory is adopted, not created*
*/
typedef struct {
    char *name;
    void *lib;
    backend_api_t api;
    backend_t *be;
    enum backend_capability capabilities;
    backend_memory_t *shared;
    uint64_t reservation;
    uint64_t guard;
    uint64_t granule;
} open_backend_t;

/*
** A component, by its namespaced name. Storage holds the rows; this is what
** load checks.
**
**     type          as storage knows it: a size
**     s             its fields or its constants, read from its text
**     storage       the storage layout every declarer stated
**     fingerprint   of the descriptor string
**     declarers     how many loaded modules declared it
**     users         how many components name it as a field's type
**     deps          the declared types its own fields name
**     zero          `size` zero bytes: what an attach with no value copies
**     replaces      at a reload, the type of this name whose rows become
**                   this one's when the reload holds
**     superseded    that the other has such a successor
**     runs          on the one that is superseded, which bytes of a value of
**                   it are which bytes of a value of its successor: nullptr
**                   if they are the same bytes
**
** schema.md, *Fingerprints*
** lifetime.md, *When a component is moved*
*/
struct declared_type {
    char *name;
    storage_type_t *type;
    schema_t s;
    enum bridge_kind kind;
    enum bridge_storage storage;
    uint32_t size;
    uint32_t align;
    uint64_t fingerprint;
    uint32_t declarers;
    uint32_t users;
    declared_type_t **deps;
    uint32_t dep_count;
    void *zero;
    declared_type_t *replaces;
    bool superseded;
    storage_run_t *runs;
    uint32_t run_count;
};

/*
** Who triggered an observer since the last barrier, and how.
*/
typedef struct {
    uint64_t id;
    enum bridge_event events;
} pending_t;

/*
** What the conflict graph reads of a term: the component, or nullptr where
** the term reads nothing, and whether it is written.
*/
typedef struct {
    declared_type_t *touches;
    bool writes;
} system_term_t;

/*
** A system, or an observer: a query and a function either way. An observer's
** phase is the one past the deployment's last, which is the barrier.
**
**     full          how [order] names it: the module's stem, then the name
**     matched       its terms, as storage holds them. nullptr with no terms
**     asked         what it asked storage for, kept so that a system taken
**                   out of storage for the length of a reload can be put
**                   back as it was
**     level         within its phase: a level runs before the next
**     parked        out of the schedule while a reload decides its fate
**     pending       who triggered it since the last barrier
**     when          of the dispatch in progress
**     turn          and where it comes among all that ran before the barrier
**     failed, why   how it ended, said after the join
**     failures      its own count
**     changes       the components it stated that it attaches or detaches.
**                   They lie after the terms, in the same block
**     term          `terms` of them
**
** schedule.md, *Order within a phase is never implicit*
** failure.md, *The budget*
*/
typedef struct {
    module_t *m;
    char *name;
    char *full;
    uint32_t phase;
    storage_terms_t *matched;
    storage_term_t *asked;
    enum bridge_event events;
    uint32_t terms;
    backend_func_t *fn;
    uint32_t level;
    int parked;
    int observer;
    pending_t *pending;
    uint32_t pending_count;
    uint32_t pending_room;
    uint32_t when;
    uint32_t turn;
    int failed;
    char why[256];
    int64_t failures;
    declared_type_t **changes;
    uint32_t change_count;
    system_term_t term[];
} bridge_system_t;

/*
** Where in a module's queue the changes of one turn begin.
*/
typedef struct {
    uint32_t turn;
    uint32_t at;
} module_mark_t;

/*
** A loaded module.
**
**     stem          the identity without its version
**     memory        its private block, as its backend sees it
**     priv          the private block. A module can write all of it, so
**                   every offset Bridge acts on is kept here and
**                   never read back from the header
**     spans         the span region: query slots, column_base, spans
**     published, had_triggers, spans_end
**                   what storage last wrote there: whether it has, whether
**                   an observer's list was not empty, where its spans end
**     declared, policy, may_write
**                   by slot: the component, what it stated for unload, and
**                   whether its grants let it write the component. Worked
**                   out once, at declare: an attach asks it every time
**     systems, observers
**                   by slot
**
** What is set aside for it alone. Only its own thread touches any of it.
**
**     ids           ids it may hand out, taken from storage at a barrier
**     ids_short     the entity ceiling cut its last refill short
**     arena_at, arena_left, arena_share
**                   its share of the persistent arena
**     owned         the ranges of the arena that are its share: one from its
**                   load, and one more for each reload that could not add to
**                   the last where it stood. `alloc` moves to the next once
**                   what is asked no longer fits in the one before. They are
**                   what goes back to the arena at unload, if nothing left
**                   alive points into them
**     queue         what it asked to have changed, applied at the next
**                   barrier in the order it asked
**     read, write   grants
**     phase_from, phase_to
**                   the phases it names that the deployment calls otherwise,
**                   and which of the deployment's each is
**     thread        the one that loaded it
**     max_failures, failures
**                   each system has a count of its own. `failures` is the
**                   module's, for what is nobody's system: a barrier that
**                   could not place an entity it changed raises it, and one
**                   that placed all it changed lowers it
**     changed_now, unplaced_now
**                   at this barrier
**     reached       the system whose count reached the number, if one did
**     left          how many changes it asked were not applied at this
**                   barrier, for want of a table and for want of columns
**     writable      the types it may write, as storage numbers them
**     barred        how many entities it asked to destroy at this barrier
**                   that carry none of them
**     config        the component its settings seeded
**     text_at, text_room
**                   where their text is
**     running       what is running in it now, or nullptr for a lifecycle
**                   call, and `turn` its turn
**     marks         where in the queue each turn's changes begin: one for
**                   every call that queued something, so the barrier can
**                   apply the queues in the order the calls ran
**     reloads       the instance this one is to take the place of
**     losses        what a reload accepts to lose
**     loss_rows     by loss: how many rows lost something under that name,
**                   plus one. Zero: nothing was
**     refused, why  an entry refused during the call in progress, and why
**
** threads.md, *Threads*
** budgets.md, *What running out does*
** strings.md, *It is given back when nothing points into it*
*/
struct module {
    bridge_t *b;
    char *identity;
    char *ns;
    char *stem;
    open_backend_t *on;
    backend_module_t *image;
    backend_instance_t *instance;
    backend_memory_t *memory;
    uint8_t *priv;
    size_t priv_len;
    size_t priv_reserve;
    uint32_t scratch;
    uint32_t scratch_len;
    uint32_t ring;
    uint32_t ring_len;
    uint32_t ring_tail;
    uint32_t spans;
    uint32_t spans_len;
    bool published;
    bool had_triggers;
    uint32_t spans_end;
    declared_type_t **declared;
    enum bridge_on_unload *policy;
    bool *may_write;
    uint32_t declared_count;
    bridge_system_t **systems;
    uint32_t system_count;
    bridge_system_t **observers;
    uint32_t observer_count;
    int64_t work_limit;
    uint64_t *ids;
    uint32_t id_count;
    uint32_t id_next;
    uint32_t id_room;
    int ids_short;
    int64_t spawns;
    int64_t persistent;
    uint32_t arena_at;
    uint32_t arena_left;
    uint32_t arena_share;
    range_t *owned;
    uint32_t own_count;
    uint32_t own_next;
    uint8_t *queue;
    uint32_t queue_len;
    uint32_t queue_room;
    char **read;
    char **write;
    uint32_t read_count;
    uint32_t write_count;
    char **phase_from;
    uint32_t *phase_to;
    uint32_t phase_names;
    int pinned;
    uint64_t thread;
    int hot_reload;
    enum bridge_on_error on_error;
    backend_func_t *shutdown;
    int64_t shutdown_timeout;
    int64_t max_failures;
    int64_t failures;
    int changed_now;
    int unplaced_now;
    const void *reached;
    uint32_t left[2];
    uint64_t *writable;
    uint32_t barred;
    declared_type_t *config;
    uint32_t text_at;
    uint32_t text_room;
    int in_init;
    int stopped;
    const void *running;
    uint32_t turn;
    module_mark_t *marks;
    uint32_t mark_count;
    uint32_t mark_room;
    uint32_t mark_at;
    module_t *reloads;
    const char *const *losses;
    uint32_t loss_count;
    uint32_t *loss_rows;
    int refused;
    char why[256];
    module_t *next;
};

/*
** One queued change: `size` bytes of value follow it, padded to eight.
*/
enum change_op : uint32_t {
    CHANGE_DESTROY = 1,
    CHANGE_ATTACH = 2,
    CHANGE_DETACH = 3
};

typedef struct {
    uint64_t id;
    declared_type_t *c;
    uint32_t size;
    enum change_op op;
} change_t;

/*
** A turn: observers of the last barrier, then lifecycle calls, then the
** phase's systems, each in the order it ran.
*/
constexpr uint32_t TURN_LIFECYCLE = 1u << 16;
constexpr uint32_t TURN_SYSTEM = 2u << 16;

/*
** Where an unloaded module's settings text was.
*/
typedef struct {
    char *stem;
    uint32_t at;
    uint32_t room;
} kept_text_t;

/*
** A deck.
**
**     reservation, guard, granule
**                   of the shared block: the address space every block is
**                   allocated with, the largest any backend stated
**     stopped_by, stopped_stem
**                   why nothing runs any more, and the module whose return
**                   lifts it: nullptr while the deck runs. They point at
**                   the two arrays after them, since a deck is stopped at
**                   a barrier, where nothing is allocated
**     turns         lifecycle calls since the last barrier
**     parked        systems out of storage while their module reloads
**     arena_ranges  how many ranges of the arena modules hold: its list of
**                   gaps has room for one more than that, found when a range
**                   is taken, so that giving one back never asks for memory
**     run_jobs, run_handed, run_lists, run_callers
**                   what a phase's dispatch works in, [budget] systems long
**     build_in, build_bits
**                   what the schedule is worked out in, when a module comes
**                   or goes: a load, a reload and an unload ask for no
**                   memory to do it, so none of them can fail there
**     before, after
**                   [order], pair by pair
**     shared        the shared block: eight bytes nothing is at, so that
**                   offset 0 is none; the entity index, the persistent
**                   arena, the columns
**     sparse_sets, sparse
**                   [budget] sparse_sets, and where their indexes begin in
**                   the shared memory: one after another, each a u32 for
**                   every entity there may be
**     direct_indexed, direct
**                   [budget] direct_indexed, and where their bits begin: one
**                   after another, each a bit for every entity there may be,
**                   in 64-bit words
**     arena_space, column_space
**                   what of each is handed out
**     layout_moved  a module came or went, or a declaration changed
**     tables        [budget] tables: how many may exist at once
**     max_types, max_systems, max_terms
**                   [budget] types, systems and terms
**     storage       who keeps the rows: storage.h
**     kept          where an unloaded module's settings text was
**     systems       every module's, in registration order
**
** budgets.md, *Nothing is allocated in a phase*
*/
struct bridge {
    bridge_sink_fn_t sink;
    void *ud;
    open_backend_t *backends;
    uint32_t backend_count;
    uint64_t reservation;
    uint64_t guard;
    uint64_t granule;
    char *stopped_by;
    char *stopped_stem;
    char stopped_why[256];
    char stopped_name[256];
    char **phases;
    uint32_t phase_count;
    uint32_t barriers;
    uint32_t turns;
    uint32_t parked;
    uint32_t arena_ranges;
    void *run_jobs;
    void **run_handed;
    void **run_lists;
    uint8_t *run_callers;
    void **build_in;
    uint64_t *build_bits;
    char **before;
    char **after;
    uint32_t order_count;
    pool_t *pool;
    uint8_t *shared;
    size_t shared_len;
    size_t shared_reserve;
    bridge_entity_t *index;
    uint32_t entities;
    uint32_t arena;
    uint32_t arena_len;
    uint32_t columns;
    uint32_t columns_len;
    uint32_t sparse_sets;
    uint32_t sparse;
    uint32_t direct_indexed;
    uint32_t direct;
    region_t arena_space;
    region_t column_space;
    int layout_moved;
    uint32_t tables;
    uint32_t max_types;
    uint32_t max_systems;
    uint32_t max_terms;
    storage_t *storage;
    declared_type_t **types;
    uint32_t type_count;
    kept_text_t *kept;
    uint32_t kept_count;
    bridge_system_t **systems;
    uint32_t system_count;
    module_t *modules;
    uint32_t module_count;
    uint32_t max_modules;
};

/*
** module_discard
**
** Frees a module and what it held.
*/
void module_discard(bridge_t *b, module_t *m);

/*
** module_shut_down
**
** Runs a module's shutdown, with what it asked for and what it said.
*/
void module_shut_down(bridge_t *b, module_t *m);

    #include "say.h"
    #include "schedule.h"
    #include "bridge_inside.h"
    #include "block.h"
    #include "abi.h"

#endif

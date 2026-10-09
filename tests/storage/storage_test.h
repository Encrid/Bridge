/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test.h -- storage: what it does, said out loud,
** and what it costs. From inside, for the barrier alone: the program is
** linked with the library's objects.
**
**     storage changes     every structural change a module can ask for, one
**                         at a time, and what a system and an observer see
**                         after each
**     storage tables      ten thousand tables
**     storage stress      four modules at once on four threads, eight
**                         million changes, each keeping its own account of
**                         what should be there and checking it
**     storage stress1     the same on one thread, which must say the same,
**                         word for word
**     storage notables    [budget] tables used up: the entity is left as it
**     storage nocolumns   was, and it is a failure of the module that asked.
**                         The same for [budget] columns
**     storage scribble    a module writes over what storage publishes in
**                         the shared block
**     storage strip       a component stripped from every entity, a table
**     storage stripfull   at a time: into the table of what is left, or
**                         where it stands. And with no room to merge
**     storage measure     nanoseconds for each kind of change, at a barrier
**     storage weigh       what the storage layouts cost where each is
**                         weakest
**     storage baseline    what the work costs that no storage layout is
**                         made for, in tables alone
**
** A case that is compared with a transcript decides nothing itself:
** build.py holds what it must say, and that is the same whatever stores the
** rows. A measurement passes by running.
**
** schedule.md, *What a pass depends on*
*/

#ifndef TEST_STORAGE_TEST_H
    #define TEST_STORAGE_TEST_H

    #include "inside.h"

constexpr uint32_t STORAGE_TEST_ROWS = 204'800;
constexpr uint32_t STORAGE_TEST_WEIGH_ROUNDS = 8;
constexpr uint32_t STORAGE_TEST_OPS = 10;
constexpr uint32_t STORAGE_TEST_MEASURE_ROUNDS = 6;

/*
** How a case ended.
**
**     DONE          main destroys the deck and says "alive"
**     LEFT          main says "alive", and the deck is left as it is
**     FAILED        the case said why, and the program answers 1
*/
enum storage_test_end : int {
    STORAGE_TEST_DONE,
    STORAGE_TEST_LEFT,
    STORAGE_TEST_FAILED
};

/*
** What main owns and every case is handed. It is what every sink is handed
** too.
**
**     cfg           the deployment the deck was created from: a case that
**                   makes a deck of its own changes it first
**     b             the deck. The one main made, until a case replaces it
**     what          the case's name, as the command line said it
**     more          the argument after it, or nullptr
**     said          what a system last said it read: kept, so that a timing
**                   is of a loop that ran
**     said_memory   how many records said that memory ran out
**     called        what the systems of the phase last timed cost, apart
**                   from its barrier
*/
typedef struct {
    bridge_config_t cfg;
    bridge_t *b;
    const char *what;
    const char *more;
    uint32_t said[2];
    uint32_t said_memory;
    uint64_t called;
} storage_test_t;

/*
** What the deployment of a case differs by. `asks_columns` is whether the
** argument after the case's name, when there is one, states the columns.
*/
typedef struct {
    const bridge_backend_t *backends;
    uint32_t backend_count;
    uint32_t threads;
    uint32_t tables;
    uint64_t columns;
    int64_t sparse_sets;
    int64_t direct_indexed;
    bool asks_columns;
} storage_test_room_t;

/*
** A case: its name on the command line, what runs it, and its deployment.
*/
typedef struct {
    const char *name;
    enum storage_test_end (*run)(storage_test_t *t);
    const storage_test_room_t *room;
} storage_test_case_t;

/*
** The library's calls to the allocator, in the build that counts them.
**
**     calls         every call, a free as well
**     asked         the calls that ask for memory
**     fail_at       which of those is answered null, counted from one: zero
**                   is none of them
**     failed        whether one was
*/
typedef struct {
    size_t calls;
    size_t asked;
    size_t fail_at;
    bool failed;
} storage_test_allocator_t;

/*
** A case that is named for what it does last: the build it loads, and what
** is done with the component.
*/
typedef struct {
    const char *what;
    const char *path;
    enum bridge_on_unload policy;
} storage_test_variant_t;

/*
** A module reloaded as a build that states st:X another way: the two
** builds, what the transcript calls it, and the losses that are accepted.
*/
typedef struct {
    const char *from;
    const char *to;
    const char *what;
    const char *const *losses;
    int64_t loss_count;
} storage_test_relaid_t;

/*
** A module reloaded as a build that keeps st:X in another storage layout:
** the two layouts, as enum bridge_storage numbers them, the phase run
** before the reload or "", and the losses that are accepted.
*/
typedef struct {
    uint32_t from;
    uint32_t to;
    const char *prepare;
    const char *const *losses;
    int64_t loss_count;
} storage_test_moved_t;

/*
** A build to weigh: how many bytes an entity of it carries, and how many
** entities it makes.
*/
typedef struct {
    const char *path;
    uint32_t bytes;
    uint32_t count;
} storage_test_carried_t;

/*
** A build to weigh: how many tables its rows are spread over, or one row
** in how many is switched on.
*/
typedef struct {
    const char *path;
    uint32_t share;
} storage_test_spread_t;

/*
** A phase that is timed, and what its line calls it. A phase that only
** undoes the one before has nullptr, and no line.
*/
typedef struct {
    const char *phase;
    const char *what;
} storage_test_op_t;

/*
** What each of the changes that are measured cost, in nanoseconds for all
** the entities: at its first barrier, at its best one, and the best of the
** module's own calls.
*/
typedef struct {
    uint64_t first[STORAGE_TEST_OPS];
    uint64_t best[STORAGE_TEST_OPS];
    uint64_t calls[STORAGE_TEST_OPS];
} storage_test_times_t;

/*
** The best of a component given and taken: the barrier of each, and the
** module's own calls of each.
*/
typedef struct {
    uint64_t on;
    uint64_t off;
    uint64_t on_call;
    uint64_t off_call;
} storage_test_change_t;

/*
** storage_test_hear, storage_test_hear_nothing
**
** Sinks: one prints a record with whose it is and its two numbers, the
** other drops it.
*/
void storage_test_hear(void *ud, const bridge_record_t *r);
void storage_test_hear_nothing(void *ud, const bridge_record_t *r);

/*
** storage_test_hear_memory
**
** A sink: counts, in the fixture, the records that say memory ran out.
*/
void storage_test_hear_memory(void *ud, const bridge_record_t *r);

/*
** storage_test_hear_counts
**
** A sink: keeps, in the fixture, the two numbers a system said, and prints
** a warning or an error that is nobody's.
*/
void storage_test_hear_counts(void *ud, const bridge_record_t *r);

/*
** storage_test_get_allocator
**
** Answers the count of the library's calls to the allocator. In the build
** that does not count them it is all zeros, and stays so.
*/
storage_test_allocator_t *storage_test_get_allocator(void);

    #ifdef COUNTED

/*
** counted_malloc, counted_calloc, counted_realloc, counted_free
**
** What the library's calls to the allocator are, in the build that counts
** them: each is counted, and the one that is to fail answers nullptr.
**
** budgets.md, *Nothing is allocated in a phase*
*/
void *counted_malloc(size_t n);
void *counted_calloc(size_t n, size_t each);
void *counted_realloc(void *p, size_t n);
void counted_free(void *p);

/*
** counted_os_reserve
**
** What the library's calls to os_reserve are, in the build that counts:
** each is counted among the allocations, and the one that is to fail
** answers nullptr.
*/
void *counted_os_reserve(size_t reserve, size_t commit);

    #endif

/*
** storage_test_state
**
** Answers a trusted plain module as a deployment must state it, with the
** identity st:store@0.1.0: one that is not reloadable, and whose first
** failure stops the deck.
*/
bridge_artifact_t storage_test_state(const char *path, int64_t spawns,
    uint32_t spans, int64_t changes);

/*
** storage_test_load, storage_test_try_load
**
** Load a module into the fixture's deck. Each answers 1 if it was
** refused, having said "FAIL: not loaded", or "not loaded" where a refusal
** is what the case is for.
*/
int storage_test_load(storage_test_t *t, const bridge_artifact_t *a);
int storage_test_try_load(storage_test_t *t, const bridge_artifact_t *a);

/*
** storage_test_load_reloadable
**
** Loads into a deck a module that is reloadable and may be reloaded. The
** program ends, saying "FAIL: not loaded", if there is no deck or the
** load is refused.
*/
void storage_test_load_reloadable(bridge_t *b, bridge_artifact_t *a);

/*
** storage_test_reopen
**
** Destroys the fixture's deck, creates another from the fixture's
** deployment with this sink, and loads a module into it. Answers 1, having
** said "FAIL: not loaded", if either is refused.
*/
int storage_test_reopen(storage_test_t *t, bridge_sink_fn_t sink,
    const bridge_artifact_t *a);

/*
** storage_test_find_variant
**
** Answers the row of a case's name, among `count` of them, or nullptr.
*/
const storage_test_variant_t *storage_test_find_variant(
    const storage_test_variant_t *rows, size_t count, const char *what);

/*
** storage_test_run, storage_test_run_all
**
** Run one phase and its barrier, named in the transcript, or each phase of
** a list that ends with nullptr. A phase that is refused is said, and so is
** one in which the library called the allocator.
**
** budgets.md, *Nothing is allocated in a phase*
*/
void storage_test_run(storage_test_t *t, const char *phase);
void storage_test_run_all(storage_test_t *t, const char *const *phases);

/*
** storage_test_drop
**
** What unload does to a component with this policy, asked of storage
** directly so that the module is still there to look; and then a barrier,
** for what was heard and for what a module reads.
**
** lifetime.md, *What unload leaves behind*
*/
void storage_test_drop(storage_test_t *t, const char *name,
    enum bridge_on_unload policy);

/*
** storage_test_time_barrier
**
** Runs the systems of one phase and then its barrier, each timed. Answers
** the nanoseconds of the barrier, and leaves those of the systems in
** `t->called`: a change is the call that asks for it and the barrier that
** applies it.
*/
uint64_t storage_test_time_barrier(storage_test_t *t, const char *phase);

/*
** storage_test_time_systems
**
** Runs the systems of one phase, timed, and then its barrier. Answers the
** nanoseconds of the systems.
*/
uint64_t storage_test_time_systems(storage_test_t *t, const char *phase);

/*
** storage_test_time_least
**
** Answers the least of the systems of a phase, timed `rounds` times.
*/
uint64_t storage_test_time_least(storage_test_t *t, const char *phase,
    uint32_t rounds);

/*
** storage_test_time_two
**
** Leaves in `least` the least of the systems of two phases, run one after
** the other four times as often as a change is weighed.
*/
void storage_test_time_two(storage_test_t *t, const char *const *phases,
    uint64_t *least);

/*
** storage_test_time_attach_detach, storage_test_time_give_take
**
** Leave in `c` the least of the barriers of attach_b and of detach_b, over
** the rounds a change is weighed. The second keeps the module's own calls
** as well, and leaves the component on at the end: its last round takes
** nothing.
*/
void storage_test_time_attach_detach(storage_test_t *t,
    storage_test_change_t *c);
void storage_test_time_give_take(storage_test_t *t, storage_test_change_t *c);

/*
** storage_test_say_held
**
** Says what the process holds, in KB: the touched pages of the block and
** everything outside it. Asked of the operating system, so that it is the
** same question whatever keeps the rows. On Windows; elsewhere it says the
** columns in use and no more.
*/
void storage_test_say_held(const bridge_t *b);

/*
** storage_test_measure_tables, storage_test_measure_stress
**
** The parts of the measure case after the first: ten thousand tables
** standing, and the stress run on this many threads. Each answers 1 if it
** could not load.
*/
int storage_test_measure_tables(storage_test_t *t);
int storage_test_measure_stress(storage_test_t *t, uint32_t threads);

/*
** storage_test_weigh_sets, storage_test_weigh_toggles,
** storage_test_weigh_direct, storage_test_weigh_tags,
** storage_test_weigh_single, storage_test_weigh_switch
**
** The parts of the weigh case after the first two, one for each storage
** layout that is not a table. Each answers 1 if it failed, having said why.
*/
int storage_test_weigh_sets(storage_test_t *t);
int storage_test_weigh_toggles(storage_test_t *t);
int storage_test_weigh_direct(storage_test_t *t);
int storage_test_weigh_tags(storage_test_t *t);
int storage_test_weigh_single(storage_test_t *t);
int storage_test_weigh_switch(storage_test_t *t);

/*
** The cases. Each is handed the fixture, with the deck main made.
*/
enum storage_test_end storage_test_look_at_shape(storage_test_t *t);
enum storage_test_end storage_test_change(storage_test_t *t);
enum storage_test_end storage_test_make_tables(storage_test_t *t);
enum storage_test_end storage_test_use_up_tables(storage_test_t *t);
enum storage_test_end storage_test_use_up_columns(storage_test_t *t);
enum storage_test_end storage_test_scribble(storage_test_t *t);
enum storage_test_end storage_test_strip(storage_test_t *t);
enum storage_test_end storage_test_strip_full(storage_test_t *t);
enum storage_test_end storage_test_tag(storage_test_t *t);
enum storage_test_end storage_test_toggle(storage_test_t *t);
enum storage_test_end storage_test_toggle_full(storage_test_t *t);
enum storage_test_end storage_test_give_single(storage_test_t *t);
enum storage_test_end storage_test_walk_single(storage_test_t *t);
enum storage_test_end storage_test_attach_single_to_entity(storage_test_t *t);
enum storage_test_end storage_test_attach_to_no_entity(storage_test_t *t);
enum storage_test_end storage_test_index_directly(storage_test_t *t);
enum storage_test_end storage_test_drop_direct(storage_test_t *t);
enum storage_test_end storage_test_join_sparse(storage_test_t *t);
enum storage_test_end storage_test_fill_sparse(storage_test_t *t);
enum storage_test_end storage_test_sweep_beside_sparse(storage_test_t *t);
enum storage_test_end storage_test_drop_sparse(storage_test_t *t);
enum storage_test_end storage_test_relay(storage_test_t *t);
enum storage_test_end storage_test_move(storage_test_t *t);
enum storage_test_end storage_test_stress(storage_test_t *t);
enum storage_test_end storage_test_measure(storage_test_t *t);
enum storage_test_end storage_test_weigh(storage_test_t *t);
enum storage_test_end storage_test_take_baseline(storage_test_t *t);
    #ifdef COUNTED
enum storage_test_end storage_test_refuse_memory(storage_test_t *t);
enum storage_test_end storage_test_refuse_relay(storage_test_t *t);
    #endif

#endif

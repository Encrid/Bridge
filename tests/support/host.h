/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/support/host.h -- what the host program of the tests is built on.
** The program is a table of cases, and a case is a function handed the
** host: what main built, and what its fixture set.
**
** A host is compiled against include/ alone and linked with the library as
** a host would be.
*/

#ifndef TEST_HOST_H
    #define TEST_HOST_H

    #include <stddef.h>

    #include "bridge.h"
    #include "bridge_abi.h"

constexpr int64_t HOST_MS = 1'000'000;

/*
** What the tests' backend is told it can keep, where a case says nothing
** else: a reload, and its library taken out of the process. They are two
** flags of bridge_backend.h, which a host does not compile.
**
** capabilities.md, *Capabilities, so the swap is honest*
*/
constexpr int64_t HOST_TESTS_KEEPS = 1 << 4 | 1 << 6;

/*
** And what it is told where a deck calls it "guarded": everything a backend
** may be asked to keep, but its library taken out of the process.
*/
constexpr int64_t HOST_TESTS_KEEPS_ALL = 63;

/*
** And the shape of a block it is told to ask for there: four gigabytes of
** address space from its start, and a length and a guard of whole pages of
** 64 kilobytes. Where a deck calls it "plain" it asks for none.
**
** backend.md, *Memory is adopted, not created*
*/
constexpr int64_t HOST_TESTS_RESERVES = INT64_C(1) << 32;
constexpr int64_t HOST_TESTS_GRANULE = 65'536;

/*
** A host under test.
**
**     what          the case that was asked for
**     arg           what the case's row hands it, or "" if it takes nothing
**     kept          what the program's fixtures keep beside the host
**     backends      the two backends there are, of which cfg names some
**     phases        "update" and "present", of which cfg names some
**     cfg           what the deck is created from: a case changes it
**                   before it opens the host
**     sink, ud      who is handed every record, and with what
**     b             the deck, once the host is open
*/
typedef struct host {
    const char *what;
    const char *arg;
    void *kept;
    bridge_backend_t backends[2];
    const char *phases[2];
    bridge_config_t cfg;
    bridge_sink_fn_t sink;
    void *ud;
    bridge_t *b;
} host_t;

/*
** A case of a program: its name on the command line, what runs it, what it
** is handed, or "" for a case that takes nothing, and the fixture that sets
** its host first.
*/
typedef struct {
    const char *name;
    void (*run)(host_t *h);
    const char *arg;
    void (*set)(host_t *h);
} host_case_t;

/*
** host_init
**
** Fills a host from the command line, with the configuration every case
** starts from: both backends, both phases, one thread, and the budgets. Its
** sink is its fixture's to set.
*/
void host_init(host_t *h, int argc, char **argv);

/*
** host_run
**
** Runs the case the command line named, of `count` cases, on the host its
** fixture set. Answers what main returns: 2, with a line, if there is no
** such case.
*/
int host_run(host_t *h, const host_case_t *cases, size_t count);

/*
** host_open
**
** Creates the host's deck. Answers 1, having said "bridge_create refused"
** and "alive", if it was refused: the case is over.
*/
[[nodiscard]] int host_open(host_t *h);

/*
** host_close
**
** Destroys the host's deck and says "alive".
*/
void host_close(host_t *h);

/*
** host_open_only
**
** A whole case: the host is opened as it is configured, and closed.
*/
void host_open_only(host_t *h);

/*
** host_print_record
**
** A sink: prints a record with whose it is and its two numbers.
*/
void host_print_record(void *ud, const bridge_record_t *r);

/*
** host_load, host_reload, host_unload
**
** Make the call and say how it ended, by the module's identity.
*/
void host_load(host_t *h, const bridge_artifact_t *a);
void host_reload(host_t *h, const bridge_artifact_t *a);
void host_unload(host_t *h, const char *identity);

/*
** host_run_frames
**
** Runs "update" and "present" `count` times, and says each that was
** refused.
*/
void host_run_frames(host_t *h, int count);

/*
** host_run_frames_unsaid
**
** Runs "update" and "present" `count` times, and says nothing.
*/
void host_run_frames_unsaid(bridge_t *b, int count);

/*
** A thread of the host's own: what it runs, with what, and the operating
** system's handle of it.
*/
typedef struct {
    void (*run)(void *arg);
    void *arg;
    void *handle;
} host_thread_t;

/*
** host_get_thread_id
**
** Answers which thread this is, as a plain test module says it: the low
** 32 bits of what the operating system calls it.
*/
uint32_t host_get_thread_id(void);

/*
** host_thread_start, host_thread_join
**
** Start a thread that calls `t->run` with `t->arg`, and wait for it to end.
*/
void host_thread_start(host_thread_t *t);
void host_thread_join(host_thread_t *t);

/*
** host_is_library_loaded
**
** Answers whether a library is in this process, asked of the operating
** system and of nothing of ours.
*/
bool host_is_library_loaded(const char *path);

/*
** host_state_plain
**
** States a plain module as a deployment must: trusted, every limit said to
** be none, and every other key said. It spawns nothing unless the case says
** so.
*/
void host_name_module(bridge_artifact_t *a, const char *file);
void host_state_built(bridge_artifact_t *a, const char *identity,
    const char *name);
void host_state_plain(bridge_artifact_t *a, const char *identity,
    const char *path);

/*
** host_state_guarded
**
** States a guarded module of tests/modules, with the limits that backend can
** keep.
*/
void host_state_guarded(bridge_artifact_t *a, const char *identity,
    const char *file);

#endif

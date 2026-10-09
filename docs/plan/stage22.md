# Stage 22: the backends leave the tree

Built: *The close*. `plan.md`, stage 22, is what it is for and when it is done. This document is
the order it was done in, what was counted before it began, and each decision as it was taken. What a decision
refused is `stage22_refused.md`.

It was asked for by a host that is being designed, which takes each backend built and so has to
know where one comes from. It is built here as *From here, a host's* of `plan.md` says such work
is: as a stage of this plan with its own number.

## What there was (measured on `ffda1a1`, 2026-10-09)

    the backends         `backends/native`: 7 sources, 644 lines. `backends/wasm`: 11 sources,
                         1345 lines, and wasmtime 48.0.5 in the tree, 122 files and about
                         164 MB
    their SDKs           the native one 4 headers, the wasm one 10 files: 9 headers and
                         `bridge_wasm.wat`
    shared source        `utils/os`: 13 files, 910 lines, compiled into the library and into
                         each backend. Nothing of `src/` is in a backend
    the headers they     `bridge_backend.h`, and through the SDKs `bridge_abi.h` and
    need                 `bridge_schema.h`. No package holds the first
    the cases            484: `host` 374 rows over 155 functions in 45 files, `host-static`
                         22, `storage` 54 and 5 counted, `adopt` 10, `nocall` 2, 2 built from
                         the packages, `cla` 6, and one for each of 9 tools
    the test modules     every one is written with an SDK. For wasm, about 3,480 lines of C
                         and 14 files of wasm text. Native, about 6,670 lines, most of them
                         for `tests/storage`, whose every module is native
    of the 45 files      36 prove the library alone and need something that runs a module. 9
                         are mixed. None is about one backend only
    what only wasm       12 rows: a trap, a work limit used up, a timeout, a memory limit
    answers today
    the tests' own       three, in `tests/support`. Two are the native backend with one
    backends             object replaced
    the packages         five for a target: `bridge`, `backend_native`, `backend_wasm`,
                         `sdk_native`, `sdk_wasm`. About 190 MB a release, most of it wasmtime
    tools that name      `style.py`, `c23.py`, `licence.py`, `cover.py`, `xref.py`, `said.py`,
    them                 `deps.py`, `package.py`, `nocall.py`
    the documents        three are a backend's or its SDK's: `native.md`, `wasm.md`,
                         `wasmtime.md`, cited 73 times in 33 files. Two sections of `wasm.md`
                         are of `bridge_abi.h`, and five other documents cite them

**It is small as a backend and large as a stage.** About ten thousand lines of test modules are
written with an SDK that leaves, and all of `tests/storage` runs on native modules.

## The order

    1  the stage        this document, and stage 22 in `plan.md` with a done-when
    2  the decisions    those under *The questions*, one at a time, before a file changes
    3  the tests'       built here beside the two backends. The cases are moved onto it a
       backend          family at a time, each still passing, while the two are still there
                        to compare with
    4  the two          created when Enkrid confirms each. What leaves is put there with its
       repositories     cases, and each passes against the library as it is handed
    5  this tree        the backends, the SDKs, wasmtime and the cases that left are removed,
                        and the build, the tools, the workflows and the documents say what
                        is then true
    6  the host's       where its first stage takes each backend from
       documents
    7  close            put to Enkrid by the done-when

## The decisions

### Where a backend is written

**Each of the two in a repository of its own.** Enkrid ruled it on 2026-10-09, over a
recommendation to keep both here until the seam for a backend is released. The seam stays:
`include/bridge_backend.h`, and what the documents under `seam/backend/` say of it. What leaves
is what answers it, and what a module is written with for each.

A backend runs a module's code. That is not the library's work, which is storage, the schedule
and the three seams, and it is not a host's, which orchestrates. So it is a project.

What it costs: a change to the seam is no longer one commit. It is made here, handed, and
followed in each backend's repository. And this tree no longer shows, by its own cases, that a
backend somebody uses works with the commit at hand: each backend's cases show it, after.

### How the library is tested with no backend in its tree

**By a backend of the tests' own, here.** Enkrid ruled it on 2026-10-09, as recommended. Its
test modules are written against `bridge_abi.h`, so no SDK is needed, and a case tells it which
outcome to report: a trap, a work limit used up, a timeout, a memory limit, a refusal. How it
carries a module was first ruled otherwise, and corrected: *How a test module is carried*. The
cases that prove the native backend or the wasm one go with that backend.

The library then depends on nothing outside its tree, and an outcome is had when a case asks
for it, where today some need a wasm module made to misbehave.

What it costs: the cases of the seam that hold for any backend are then in two repositories,
and may drift.

### Whose they are, and who sees them

**Public, under the licence and the notice the library has.** Enkrid ruled it on 2026-10-09:
the same as Bridge.

### What the two are called

**`BB-Native` and `BB-Wasm`, under `Encrid`.** Enkrid ruled it on 2026-10-09. BB is for a
backend of Bridge's, and what follows is the backend. It was said as "BB as a prefix", read
aloud both ways, and confirmed as the letters.

### What a test module is written with

**A binding of the tests' own, and nothing at all where a module's shape is the subject.**
Enkrid ruled it on 2026-10-09, as recommended. The binding is under `tests/`, made from the
four headers of the native backend's SDK, with names that begin `test_` and `TEST_`. It
includes `bridge_abi.h` and `bridge_schema.h` and nothing else, so the tests still show that
the seam is enough to write a module. A module whose case is about what a module is - one with
no `init`, one whose `init` has another signature - is written bare, as the fourteen in wasm
text are today.

The rule between the two: the binding, unless the binding could not say it.

It is exposed to nobody, so `tests.md` gives it no heading. What it costs: the library holds
what began as a copy of another project's SDK, and the two will differ.

### How a case gets an outcome

**The module does it, and the tests' backend judges it against what the library set.** Enkrid
ruled it on 2026-10-09, as recommended. The tests' binding gives a module four moves: to trap,
to spend so many units of work, to wait so long, to grow by so many bytes. The backend holds
each against the limit the library set on that instance through the seam and, past it, unwinds
the module and answers `call` as a backend does. No time passes: work and waiting are counted.
A case chooses an outcome by the module it loads.

So an outcome reaches the library by the seam and by nothing else, and a case that ends a
spinning system also shows that the limit it stated is the one that was set.

What a backend is, a case has by the library it names, as today: the capabilities by a setting
of the tests' backend, and the three the tests have - one with a single entry, one that answers
another number, one that refuses - built from the tests' backend where two were built from the
native one.

### Which cases stay

**Settled by the two decisions above, and by what a case proves.** No question was put: one
answer follows.

    it proves the library, and     stays, on the tests' backend. A module that was native or
    needs a module run             wasm only because it happened to be is written with the
                                   binding
    it proves what the library     stays, by the four moves. That the wasm backend produces
    does with an outcome           the outcome is a case of that backend's, written there
    it proves a backend, or an     leaves with it: the cases named for an SDK, `adopt`,
    SDK                            `nocall`, the timing of the row accessors, a module of wasm
                                   text refused for its shape
    an entry of the seam           stays, shown on the tests' backend: that the library asks
                                   it rightly. That a backend answers it rightly is that
                                   backend's
    a file that holds several      is split by function
    kinds

Each of the 484 is written down by name, with where it went, when it is moved: *How each claim
is shown*.

### The layer over the operating system

**Copied into each repository, and made its own there.** Settled by a rule that stands: a
project copies what it needs and runs nothing from another's directory. `utils/os` depends on
nothing of the library's and says so. What it costs is three copies that may differ, of 910
lines.

### What whoever writes a backend is handed

**A second package, `seams`: `bridge_backend.h`, `bridge_abi.h`, `bridge_schema.h` and
`license`.** Enkrid ruled it on 2026-10-09, as recommended. It is for whoever writes a backend,
or what a module is written with for one. A host is handed `bridge`, which keeps the one header
a host includes. Each audience takes what it uses, as the packages were cut in the first place.

A backend's repository takes both, at one commit and by the hash of each file: `seams` to be
built, and `bridge` to run a host in its own cases.

### What the packages become

**Two from here, `bridge` and `seams`, and two from each backend's repository: its own and its
SDK's.** It follows from the decision above and was not put. An SDK's package still holds
`bridge_abi.h` and `bridge_schema.h`, which its repository has from `seams`, so that whoever
writes a module takes one package.

### What a document here says of a backend's

**Nothing by name: no document of this tree cites one of a backend's.** Settled without a
question, by which way the dependence runs: a backend is written against this tree, and this
tree against no backend. A sentence here that needs what `native.md`, `wasm.md` or `wasmtime.md`
says is reworded to what the seam asks, or leaves with them. The two sections of `wasm.md`
that are of `bridge_abi.h` - its layouts, and that it holds no content - go to `abi.md`, which
is that header's document.

### What each repository is made from

**From this one: its method, its style, the tools the code is held by, and its build, each
copied and made that repository's own.** Enkrid ruled it on 2026-10-09, as recommended. The
code that leaves was written under these rules and passes these tools, so nothing is written
anew to hold it.

Each file's notice names its own project, `BB-Native Owner` and `BB-Wasm Owner`. `license` is
the same text, and `cla.md` the same agreement with the name changed. Each has a first stage of
its own that says where its content came from, by the commit of this tree.

What it costs: the method's text is then in five repositories, kept true by hand.

### How a backend takes the library

**By a pin, and from a folder where a change is being tried.** Enkrid ruled it on 2026-10-09,
as recommended. A backend's tree states one commit of this one and the hashes of `bridge` and
`seams`, in a file `bridge.txt` at its root. Its build fetches the two from the release of that
commit and refuses a file that is not what its hash says - `host_api.md`, *What a host is
handed*.

**A change to the seam goes in this order.**

    1  on a branch here. Its two packages are written into a folder, and each backend's
       build is pointed at that folder: the change is tried against both before it lands
    2  to `main` here, which publishes
    3  each backend moves its pin, passes its cases, and publishes

A package from a folder is taken as a package is, by its statement and its hashes. Nothing of
this tree is run by a backend's build.

**What a backend publishes states the commit of the library it was built against.** The number
of the seam cannot say it: nothing is released, so the number does not move while the seam
does. A host that is handed a backend built against another commit than its library refuses
it, and that check is the host's.

What it costs: three steps where there was one commit, and between 2 and 3 the newest of each
do not go together.

### What a backend publishes, and when

**Its two packages, for each commit of its `main`, by the run that tests it.** It follows from
what each repository is made from, and was not put: it is what this tree does, copied -
`host_api.md`, *What a host is handed*.

### Whether what leaves keeps its history

**It does not: each repository begins with what left this tree, and says which commit of this
tree that was.** Enkrid ruled it on 2026-10-09, having first ruled that the history be carried.
When the tool that would carry it was to be chosen, he ruled that it is not. So each repository
is small, every commit in it is about what is there, and the reason for a line of a backend
written before this stage is read here, where the history stays whole.

This tree is not written again: its history keeps the backends up to the commit that removes
them.

What it costs: `git blame` there stops at the first commit.

### How a test module is carried

**As a library of its own, which the tests' backend opens by its path.** Enkrid ruled it on
2026-10-09, as recommended, in the place of what the decision on testing had first said: that
the test modules are compiled into the backend. That did not link. A module of several files
shares functions and variables between its files, and one source is built under several names:
compiled into one library, each of those names is there once for each build, and the linker
refuses. The first family moved passed only because its modules are one file each.

A library is a space of names of its own, so nothing meets, and it is how the build already
made every native module. The library still has no backend of another project and no SDK:
what opens a module's library is `utils/os`, which stays.

## The questions

None stands. Twelve were listed when the stage was planned: eight were put and ruled, and four
followed from those and from a rule that stands, each said as such under its heading.

## What was built here

    `tests/backend/`            the tests' backend: ten sources. It opens a module's library,
                                keeps the three limits by counting what a module says it
                                does, ends a call on a trap, and refuses an entry declared
                                otherwise than `bridge_abi.h` states, or an instance not
                                handed both memories. It is told what it keeps and the shape
                                of a block it asks for, by four settings
    `tests/binding/`            what a module of the tests is written with, in two faces: by
                                the table, with four moves of the tests' own; and by offsets,
                                over accessors that go through the same table
    `tests/modules/`            every module the cases load, written with the binding, and
                                seven written bare for a module's shape or its outcome
    `include/bridge_abi.h`      the eight entries, stated as one type
    `python build.py package`   two packages: `bridge` and `seams`
    `tests/packaged`            a host from the library's package, with the tests' backend and
                                a module compiled against the seams' package
    the tools                   `cover.py` reads the three seams' headers; `said.py` the
                                library's messages; `xref.py` lets a stage's record cite a
                                document that left; `style.py`, `c23.py`, `licence.py`,
                                `deps.py` and `package.py` name nothing that left
    the workflows               fetch zig and nothing else

## What was found on the way

**A module's library has its variables once in a process.** Two instances of one module share
them, where the wasm backend gives each instance its own. So a case that needs one module
twice, in two decks say, has it built under two names, as a native module was. What such a
case shows is then that the library keeps two decks apart; that two instances of one wasm
module are apart is that backend's to show. It was not put: the other way is every module's
variables moved into memory an instance is handed, in every module.

**The tests' backend stands beside the two while the cases are moved.** A deck of the tests
names three backends until the last case has left the other two.

**A module written for the wasm backend is not written again.** That backend's SDK is a
few dozen functions over a handful of accessors, which read and write at an offset into the two
memories. So the tests' binding has a second face: the SDK's headers as they were, over
accessors of the tests' own that do the same through the table. A module written by offsets
compiles as it was, but for the header it includes, and what its cases show is what they showed.
About 3,480 lines were copied and two of them changed, where a loop without end became work
and waiting that are counted.

**A module written by offsets is told its table before each call**, since its functions take
nothing. The table is kept once for each thread: a deck may have one library's module twice,
and two decks the same module, each called on a thread of its own.

**What a host hands as bytes is the path.** A module of the wasm backend is handed as bytes
and their length, and the tests' backend takes those bytes as the path of the module's
library, of the length stated and no longer. So the cases that hand bytes still show that
they arrive.

**A deck of the tests names the tests' backend twice: "guarded" and "plain".** It was "wasm"
and "native" while the cases were moved, so that no expected text changed for a name. Guarded,
it is told it keeps every limit and asks a block's shape; plain, it is told it keeps a reload
and its library taken out, and asks none. The names are the deployment's. The files, the
functions and the cases of the tests that said one of the two backends say one of these.

## Where each case went

Counted against the build's list: 484 when the stage began.

    left with the wasm      `bridge_load-memory-with-a-data-segment`, and the same with the
    backend, which they     static library; `module_config.memory_limit-memory-larger-at-start`,
    prove: 9                and static; `instance_get_func-export-taking-an-argument-refused`,
                            and static; `instance_get_func-export-answering-64-bits-refused`,
                            and static; `bridge_load-import-nobody-answers`
    added: 3                `instance_get_func-no-such-export-refused`, and static: what the
                            library does when a backend finds no such function, which the
                            two that left had shown by a function of another shape.
                            `backend.settings-none-where-one-is-needed-refused`: the tests'
                            backend is not made without being told what it keeps
    reworded, the same      `BACKEND_IMPORT_OTHER-a-global-nobody-answers`: a module of the
    case: 1                 tests says it imports what nobody answers, and no instance is made

478 after the modules written for the wasm backend were moved. A case whose expected text was
the wasm backend's own words - a trap, fuel used up, an interrupt - now expects the tests'
backend's, and is the same case.

**The eight entries of the ABI were named in no header of the library.** `declare`,
`add_system`, `observe`, `spawn`, `destroy`, `attach`, `detach` and `alloc` were stated in
`abi.md` and in each backend's SDK, and `tests.md` had their headings from the native SDK's
table. With the SDKs gone, forty-one cases that prove the library would have hung on nothing.
So `bridge_abi.h` states them, as `bridge_backend.h` states the entries of its seam: one
type, `bridge_entries_t`, which is their names and what each takes and answers. It is in no
memory and is no layout. It was not put: it is what the other seam's header already does.

Then, counted again: **62 cases named for a function or a macro of an SDK left with that SDK.**
What each ran is still run where a case of the library names it: most were a second name for a
case of storage or of an entry. One a document relied on was kept under the entry it proves:
`bridge_lend-scratch-given-back` is `attach-many-values-through-little-scratch`.

    `BRIDGE_DECLARE-an-enum-and-a-bitmask`, `BRIDGE_COMPONENT-declared-by-two-modules`,
    `BRIDGE_DECLARE_TAG-carried-by-two-of-three`, `BRIDGE_ADD_SYSTEM-registered-and-run`,
    `BRIDGE_OBSERVER-registered-and-told`, `BRIDGE_PUT_CHANGES-an-observer-that-attaches`,
    `BRIDGE_ATTACH-the-value-travels-with-it`, `BRIDGE_ROW-no-call-in-the-loop`,
    `BRIDGE_ROW-checked-and-unchecked-timed`, `BRIDGE_GET-a-field-read`,
    `BRIDGE_SET-a-field-written-each-frame`, `BRIDGE_IMPORT-the-entries-imported`,
    `BRIDGE_ACC-the-accessors-merged-in`, `BRIDGE_EXPORT-a-system-found-by-its-name`,
    `BRIDGE_HEADER-the-scratch-arena-found`, `bridge_get_entity-the-id-of-a-row`,
    `bridge_get_ref-own-entities-followed`, `bridge_is_alive-ids-a-sweep-left`,
    `bridge_get_sparse_index-where-the-index-is`, `bridge_get_sparse_value-the-value-of-an-id`,
    `bridge_get_single-where-the-one-row-is`, `bridge_get_cells-where-the-cells-are`,
    `bridge_get_cell-the-cell-of-an-id`, `bridge_line_t-a-line-of-text-found-at-run-time`,
    `BRIDGE_LOG-a-line-with-two-numbers`, `bridge_log-a-line-of-text-found-at-run-time`,
    `bridge_count_triggers-how-many-this-barrier`, `bridge_get_trigger-each-in-turn`,
    `bridge_get_trigger_id-who-it-was`, `bridge_get_trigger_events-enter-attach-leave`,
    `BRIDGE_STATE-a-statement-put-in-scratch`,
    `bridge_take_scratch-more-than-given-stops-the-module`, `bridge_put-bytes-copied-into-scratch`,
    `bridge_count_spans-one-table-matched`, `bridge_get_span-one-table-walked`,
    `bridge_count_rows-rows-of-a-span`, `bridge_get_entities-where-a-span-keeps-its-ids`,
    `bridge_get_base-where-a-column-begins`, `bridge_get_bits-where-a-toggle-keeps-them`,
    `bridge_is_on-counted-by-its-bit`, `bridge_turn-four-turned-off`,
    `BRIDGE_NATIVE_ATTACH-seen-by-a-wasm-observer`, `BRIDGE_NATIVE_DECLARE-an-enum-a-struct-names`,
    `BRIDGE_NATIVE_COMPONENT-declared`, `BRIDGE_NATIVE_DECLARE_TAG-carried-with-no-bytes`,
    `BRIDGE_NATIVE_ADD_SYSTEM-registered-and-run`, `BRIDGE_NATIVE_OBSERVER-registered-and-told`,
    `BRIDGE_NATIVE_PUT_CHANGES-changes-stated-and-made`,
    `bridge_native_get_query-the-query-of-a-system`, `bridge_native_get_span-two-tables-walked`,
    `bridge_api_t-entry-from-a-foreign-thread-refused`, `BRIDGE_NATIVE-system-reads-and-writes`,
    `BRIDGE_NATIVE_STATE-a-statement-put-in-scratch`,
    `bridge_native_get_header-read-through-a-pointer`,
    `bridge_native_put-bytes-copied-into-scratch`, `bridge_native_lend-a-value-lent-to-attach`,
    `bridge_native_get_single-the-one-row`, `bridge_native_get_cells-reached-by-an-id`,
    `bridge_native_get_sparse_index-reached-by-an-id`, `bridge_native_get_bits-read-and-turned`,
    `bridge_native_get_list-triggers-of-an-observer`, `bridge_native_log-a-line-with-two-numbers`

416 after them.

**The program that called a backend as the library does was the wasm backend's.** Its ten
cases stood under headings of the backend seam and showed that backend: one block in two
stores, a trap after which the store runs on, twenty thousand timed calls. It leaves with it.
What the library does at those entries is shown here by the tests' backend, which now holds
what it is handed against the headers: `define_func` refuses an entry declared with another
signature than `bridge_abi.h` states, and `instance_create` an instance that was not handed
both memories. So every load of a module of the tests shows both, and nine cases under those
headings are a load: `backend_sig_t-what-each-entry-takes-and-answers`,
`backend_entry_t-what-answers-an-import`, `backend_memory_import_t-both-memories-handed`,
`memory_release-after-the-last-instance`, `instance_create-with-both-memories`,
`define_func-called-by-a-module`, `call-a-call-that-was-ended-answered`,
`BACKEND_I32-an-argument-of-32-bits`, `BACKEND_I64-an-argument-of-64-bits`. The tenth, `adopt`,
had no heading but its own and is gone. 415.

**The check of what is said holds the library's messages, and no backend's.** It read the
two backends' sources beside the library's; theirs are held where they now are.

**The build makes no backend that a deployment runs, and needs zig and Python.** The two
backends, their SDKs, their test modules, the engine one of them wraps and the tools that
assembled wasm are not built and not in the tree: `backends/`, the modules of each under
`tests/modules/`, `tests/adopt`, `tools/nocall.py`, and the three documents under
`docs/backends/` with their records. What is handed is two packages, `bridge` and `seams`, and
a host, the tests' backend and a module are built from them and from no header of the tree.

**A record of a stage still cites a document that left.** `native.md`, `wasm.md` and
`wasmtime.md` are cited by the records of ten stages, which say what was true then. The check
of citations lets a record do so and nothing else: `tools/xref.py`.

## What is shown so far, and what is not

Of this tree, on 2026-10-09, at the commit after the backends were removed:

    every case              415 of 415 pass, optimised and debug, alone and six at once,
                            where the code is written: Windows on x86-64
    every other target      the five compile and link with no error: Linux and macOS on
                            both processors, and Windows on arm64. They are not run here
    what is imported        on each of the six, what the library imports is what
                            `dependencies.md` lists: 45, 36, 36, 38, 38 and 46
    the two packages        written, and held by `tools/package.py`; a host, the tests'
                            backend and a module built from them and from no header of the
                            tree, and run

    the tools               each fails on a change that should fail it, on a clone of the
                            tree: `cover` on a heading no header exposes and on a case
                            taken from under its heading; `xref` on a document that is not
                            there and on one that left, cited outside a record; `style` on
                            a line too long in the tests' backend and on a variable of the
                            binding that is not constant; `c23` on an older spelling;
                            `licence` on a seam's header that does not say what it is
                            under; `deps` on a tool nothing asks for; `package` on a
                            package the document does not name; `said` on a message no
                            case makes appear

    not shown               the five other targets run, which is the workflows' for what
                            reaches `main`

Of the two repositories, on the same day. Both were created at Enkrid's word, private, and
each holds on its branch `dev` what left this tree at `ffda1a1`, put where that project keeps
it, with a build of its own that is handed the folder of this tree's two packages and reads
no other path of it:

    `BB-Native`             the backend, its SDK, 54 cases: 54 pass. 39 are a host's and
                            one is of a program that calls the backend as the library
                            does; 14 are its tools. Every name of the SDK has a case. The
                            ten that read a layout through the SDK were run here by a
                            program that reaches inside the library; there they are a
                            host's, which loads the same modules and runs their phases
    `BB-Wasm`               the backend, its SDK, wasmtime, 324 cases: 324 pass. 298 are
                            a host's: the cases named for its SDK, and every case of the
                            host program that ran on that backend alone. Ten are of the
                            program that calls it as the library does, two read a
                            compiled module, and 14 are its tools. So that backend is
                            shown against this library by more than what left with it
    each of the two         its cases pass optimised and debug, alone and six at once, and
                            from a fresh clone of its repository, with the folder of this
                            tree's packages and no other path of it. For `BB-Wasm` the
                            clone was handed the Binaryen the build had already fetched
    their method            each holds its own: `contributing.md`, `style.md`, `plan.md`,
                            `tests.md`, `dependencies.md`, `licence.md`, `cla.md`, and the
                            tools, which are cases there as they are here. A document of
                            theirs cites one of this tree's by its name and a heading,
                            held against a list of this tree's headings at the commit the
                            backend was written against
    their tools             each fails on a change that should fail it, on a clone: the
                            same eleven changes as here, made to that tree, and for
                            `deps` an import nothing asks for as well. `nocall`, which is
                            `BB-Wasm`'s, fails on a function that calls
    every other target      each backend compiles and links for the five, and on each of
                            the six what it imports is what its `dependencies.md` lists:
                            41, 42, 25, 24, 27 and 25 for `BB-Native`, 89, 90, 74, 74, 75
                            and 75 for `BB-Wasm`, in the order Windows, Linux, macOS, and
                            x86-64 before arm64 in each. They are not run here
    what was removed        from each backend's tests, what only the other backend's
                            cases called: 15 functions, seven files with no function left
                            in them, and the counts a native module fed in `BB-Wasm`
    the three together      a host built from the library's package, the two backends'
                            packages and a module compiled against each SDK's package, and
                            from nothing else of the three trees, loads a native module and
                            a wasm one and runs them three passes. Each backend's package
                            states the commit of this tree it was built against

    this tree's `main`      merged at Enkrid's word, at `bb22256a976a`. Its three runs passed,
                            and its prerelease holds twelve archives: `bridge` and `seams`
                            for each of the six targets
    each one's pin          a file at its root states that commit and the hash of each of
                            the twelve. Its build fetches the two of its target once, holds
                            each archive before it is opened, and passes its cases from a
                            fresh clone with no folder named: 55 for `BB-Native`, 325 for
                            `BB-Wasm`, each having gained the case that refuses an archive
                            that is not the one stated
    what each publishes     its `main` was made at Enkrid's word, and its three runs,
                            started by hand, passed: every case once and six at once on
                            Linux and macOS on both processors and on Windows on x86-64,
                            with Windows on arm64 compiled, linked and packaged. Each
                            prerelease holds twelve archives, and the backend's statement
                            in each names its commit and this tree's
    the three, published    a host built from the three prereleases' archives for Windows
                            on x86-64, and from no build folder of any of the three, loads a
                            native module and a wasm one and runs them three passes. The
                            host's source and the two modules' are this tree's as they were
                            at `ffda1a1`, and Binaryen is the one a build had fetched

    not shown               in each repository: a run started by a push, and a pull
                            request's. And a host built so on a system other than Windows

## How each claim is shown

    each case accounted for   a table here: each of the 484 by name, where it is after the
                              stage, or why it is gone, counted against the build's own list
                              before and after
    the library alone         every case, optimised and debug, alone and six at once, with no
                              `backends/` in the tree
    no name left              a search of the tree for `backends/`, `bridge_native`,
                              `bridge_wasm` and `wasmtime`: each hit is a record of a stage,
                              or a sentence on what a backend is
    the tools still fail      each shown to fail on a change that should fail it, on a copy
    each backend              its own command, on a fresh copy of its repository, with the
                              library as it is handed and nothing else of this tree
    the three together        a host built from what the three publish, run
    every other target        compiles and links

## The close

Put to Enkrid on 2026-10-09, who ruled it done.

    no file of a backend or of an SDK is in the tree,     met: a search of the tree, where
    and no tool or document of it names one as its own    each hit is a record of a stage or
                                                          a sentence on what a backend is
    every case passes on the tests' backend alone         met: 415 of 415, optimised and
                                                          debug, alone and six at once, and
                                                          by the run for `main` on the five
                                                          platforms that are run
    each of the two repositories builds its backend       met: 55 and 325 cases, each from a
    and its SDK from its own tree and from what the       fresh clone with its pin of this
    library hands, and passes the cases that left         tree and nothing else of it
    each publishes what it makes for a commit             met: a prerelease of each one's
                                                          `main`, twelve archives
    a host built from what the three publish loads a      met: on Windows on x86-64, from
    native module and a wasm one and runs them            the three prereleases' archives

**What is not shown.** A host built so on another system than Windows. And in each of the two
repositories a run started by a push, theirs having been started by hand, and a pull request's.

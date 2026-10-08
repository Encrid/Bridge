# Host API

What was weighed here and refused is `host_api_refused.md`, *What was refused: Host API*.

The surface any host links against. What Bridge requires of the values a host hands over is
`config.md`.

The arrangement is idTech3's: the engine is `qcommon/`, and `win32/win_main.c:1212` calls `Com_Init`
then loops on `Com_Frame`.

This is the third seam. `module.md` and `bridge_abi.h` are Bridge to modules, with each
backend's SDK beside that backend - `readme.md`, *Three layers, three seams*;
`backend.md` is Bridge to a backend; this is **host to Bridge**. Everything in this
folder is `bridge.h`, which is the only header a host includes - one per seam, `readme.md`, *Three
layers, three seams*.

## A host links a built library

**A host does not compile Bridge. It links what the build made, and it may be written in
any language that can call C.** So the seam is judged by that and not by which compiler reads
`bridge.h`: the header is the binding for a host in C, and one binding among however many.

The build makes two files, and they are the same objects:

    bridge.dll, bridge.so, bridge.dylib    a shared library: the seven functions of *The
                                           surface* are what it exports, and nothing else of
                                           the library can be reached through it
    bridge.a                               a static library, for a host that is to be one file

**The shared one is what makes the seam a fact.** A host that links it cannot name anything of
the library's inside, because the linker has nothing to give it. The host program of the tests,
`tests/host`, is compiled against `include/` alone and linked with it, so every run of the tests
goes through it; and it is linked a second time with the static one, and runs again the cases
of what a host does first.

**Every struct of the seam is laid out where it is stated**, so that a binding in another
language is written against numbers and checked by them. `bridge.h` asserts the size of each
struct and the offset of every member, and leaves no padding to a compiler: a gap is a member
named `pad`, numbered where a struct has several, written as zero and never read. It is the rule
`bridge_abi.h` has for what a module reads - `wasm.md`, *The ABI layouts* - and `bridge_backend.h`
has it too, for a backend that is not in C. A pointer and a `size_t` are eight bytes: no build of
the library exists for any other. A host in C writes these structs by member name, and never
sees a `pad`.

**To a C++ compiler the three seam headers are C.** They open `extern "C"`, so a host or a
backend in C++ links the names as they are exported. They are C23 otherwise, and what they use
of it C++ has too: an enum of a stated width, a typed constant, `static_assert`. The wasm SDK
is not for C++: it chooses an accessor by `_Generic`, which C++ does not have. Read as
C++20 by a compiler that is not this project's, all three parse. No C++ host has linked and run
them.

### The seams for a language that is not C

It is not built.

The three seams in a form that needs no C parser at all: what a host, a module or a backend in
another language reads to know each layout, each constant and each entry. And, for a module, a
binding in its own language, which is its backend's to provide as the two there are provide C's.

## What a host is handed

**Packages.** A package is a folder of built files set apart for somebody else to take, and
the build writes five for a target, under `packages/` of the folder it builds into:

    `bridge`           the library: its shared file, its static file, `bridge.h`, which is
                       the one header a host includes, and `license`
    `backend_native`   the native backend's library, and `license`
    `backend_wasm`     the wasm backend's library, wasmtime's shared library, which that
                       backend loads from beside itself, `license`, and wasmtime's own as
                       `license_wasmtime`
    `sdk_native`       what a native module is written with: `bridge_abi.h`,
                       `bridge_schema.h`, the files of that backend's `sdk/`, and `license`
    `sdk_wasm`         the same for a wasm module

**A host takes the library's, and a backend's for each backend its deployment names.** A
backend is a library of its own that a deck opens by the path a deployment states - `config.md`,
*The keys* - and a deck is not created without one, so the library's package alone runs
nothing. An SDK's package is for whoever writes a module, which a host that only runs other
people's modules never is. The backends the tests build for themselves are in no package.

**Each package states itself, in `package.txt`.** Its name, the commit of Bridge it was built
from, the target, whether the cases were run on that target and passed in the build that made
it, and then the SHA-256 and the name of each file it holds. A host's build that takes a
package reads that file and refuses one whose file is not what the hash says. It is not a
manifest, which is a module's - `module.md`, *The manifest*.

**`python build.py package` makes them to be taken.** It builds, runs every case where the
target is the machine's own, writes `run yes` where they passed, and makes of each package one
archive named for the package and the target. For a target of another machine it builds, and
the statement says `run no`.

**They are published for each commit of `main`**, by the run that tests it: a release of the
repository named `main-` and the first twelve characters of the commit, which holds the
archives of each target whose job passed. It is named for a commit and claims no version.
Windows on arm64 is published by the job of Windows on x86-64, with `run no` - `build.md`,
*Platforms*. A job that fails publishes nothing, so a release may lack a target.

**What a host is handed is shown to be enough**, by a host and two modules that are built
from the packages and from nothing else of the tree: `tests/packaged`, which includes
`bridge.h` out of the library's package and links that library, shared and then static, and
`tests/modules/packaged`, a native module and a wasm one, each compiled against its SDK's
package. `tools/package.py` holds what each package holds against this section.

**One thing `bridge.h` does not give a host: the number of the module seam.** A host states it
for each module it loads, in `bridge_module_config_t`, and it is the module's to say, in its
manifest - *A seam has a number*, below. Until a manifest is read for a host - `module.md`,
*The parser of a manifest* - a host is told that number by whoever built the module.

## A seam has a number

**Each of the three seams has a number, and a side that states another is refused before
anything else of it is read.** A host, a backend and a module are each built at their own time,
and since the library is a file a host links, at a time other than the library's. A struct read
at offsets the other side does not use is not an error anybody is told of; so it is made one.

    the host seam       BRIDGE_HOST_ABI   a host states it first in `bridge_config_t`, and
                                          `bridge_create` refuses another, by both numbers
    the backend seam    BACKEND_ABI       a backend's library answers it from its first
                                          entry, and one that answers another is not asked
                                          for the rest - `backend.md`, *What a backend is
                                          asked for*
    the module seam     BRIDGE_ABI        a module states it in its manifest, and a load
                                          refuses another - `wasm.md`, *The ABI layouts*

`tests/host`, cases `config.abi-another-refused`, `config.abi-unset-refused` and
`backend_get_abi-another-refused`. Each number is where it can always be found: first in the
struct, the first entry.

**All three are 1, and stay 1 until there is a release.** A release is a version of this
repository that is tagged and handed to something that is not rebuilt with it. None exists, so
until one does a layout changes freely and no number moves. From the first release on, a change
to what a seam lays out or means raises that seam's number, and only that seam's.

## Nothing a host hands over is parsed

Which side owns what is `split.md`, *The split*. At this seam it comes to one thing: **nothing a
host hands over is parsed** - not TOML, not object files, not wasm sections. The one text the
library reads is a module's, the descriptor of a type: `schema.md`, *There is no schema format*.
`bridge_create` takes a struct; `bridge_load` takes a struct plus bytes and a path, and passes those
two straight to the module's backend without inspecting either - `lifecycle.md`, *The runtime
lifecycle*.

**The one kind of file Bridge opens is a backend library.** Config names each backend and its
library, and `bridge_create` opens them - `create.md`, *Create*. A backend is Bridge's own
machinery rather than content, and a deployment that names one it cannot open has not got an
orchestrator to start.

So an embedder with its own config system - a database, compiled-in constants, another format -
hands over data rather than being made to write a file for Bridge to read. And the three platform
parsers that would otherwise sit in the library do not: `module.md`, *The parser of a manifest*.

**No discovery either.** Finding modules is filesystem work, so the host finds them and calls
`bridge_load` for each.

## The surface

Seven functions, and `include/bridge.h` is the statement of each: what it takes, what it answers
and when it may be called. A document says why each is as it is.

    bridge_create, bridge_destroy       a deck made, and unmade: `create.md`
    bridge_run_phase                    a phase run, then its barrier: `loop.md`
    bridge_load, bridge_unload,         a module in, out, and replaced, between phases only:
    bridge_reload                       `lifecycle.md`
    bridge_list_modules                 what is loaded, pulled: `introspection.md`

**What is not built is not in this list, nor in the data below**: the pull of `profile.md`, *The
profile*, and the two keys of `validation.md`, *The checksum pass, when asked*.

Every answer above the pull is `[[nodiscard]]`: a status a host drops is one the compiler
points at, and `bridge_destroy`'s is the one that leaks a deck when dropped - `create.md`,
*Destroy*.

## The data

Ten structs, each laid out in `include/bridge.h`, which says what every member is.

    bridge_config_t           a deployment: the seam's number, its backends, its phases, the
                              pairs of `[order]`, how many threads, and the budget
    bridge_backend_t          a backend: its name, its library, and settings of its own
    bridge_budget_t           how many of each thing there may be at once, and how many bytes
    bridge_order_t            two systems that conflict, in the order they are to run
    bridge_setting_t          a key, and a value of one of four kinds

    bridge_module_config_t    what a module is to the deployment: every key of `config.md`,
                              *The keys*, stated
    bridge_phase_name_t       a phase a module names, and the deployment's phase that it is
    bridge_artifact_t         a module to load: its config, the backend that runs it, and
                              its bytes and its path, both handed to that backend, which
                              refuses if the one it needs is null

    bridge_record_t           what comes back out, handed to the sink: `create.md`, *The sink*
    bridge_module_info_t      a loaded module, and whether it still runs

**A phase is a string, not a struct.** It has one property and that is its name - `schedule.md`, *A
phase is a name and nothing else* - which also disposes of a collision: a typedef and a function
share one namespace in C, and `bridge_run_phase` is the call.

**And `bridge_run_phase` takes that name, every time.** It is looked for among the deployment's phases
by comparing, which is a few nanoseconds for the handful there are, in a call that takes about a
hundred with nothing to do, made once a phase and never once a row. What that buys: a name is what
any language hands to C with no binding, the call says which phase it runs, and a wrong one is
refused by its name.

**The host resolves its own sources** - a manifest, a deployment's file, whatever it has - and
hands one flat struct per module. Bridge has no notion of where a value came from, which is why
nothing here records a source.

**Every limit here carries its period, because a number without one means nothing** - `limits.md`,
*What bounds a run*. One is per invocation, three are per call, and one is a ceiling.

**A backend refuses a setting it does not know, by name**, and `bridge_create` fails with it: a key
nobody reads is a typo nobody would find, the argument `config.md`, *A module's own settings are a
component*, makes for a module's. Neither backend built so far has a setting, so each refuses any.

**A backend's `settings` are its own** - the cache directory, the pooling limits -
and Bridge passes them to `backend_create` unread: `backend.md`, *What a backend is asked for*.

**A module's `settings` seed a component, and nothing more** - `config.md`, *A module's own
settings are a component*, for why that means there is no call to reconfigure here.

### Nothing here is an ABI layout

The structs coming back out carry **resolved values**, never the shapes Bridge keeps
internally. That is *Nothing a host hands over is parsed* pointing the other way: no formats in, no internal
structs out, and a host that can be replaced without touching the library.

It is not a preference. The record a module writes into its ring - `log.md`, *The log* -
stores `text` as an offset into a wasm memory a host cannot address, `system` as a slot number, and
no identity at all, since the ring it sits in is what says whose it is. Handed over as-is it would
tell a host nothing it could print. Three of its fields have to be resolved, which is why these are
two structs and not one.

What it costs: two definitions of nearly the same thing, kept in step by hand. What it buys: the
ring's stride is an internal decision - change it and no host recompiles.

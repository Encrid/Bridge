# Stage 21: what a host is handed

Built: *The close*. `plan.md`, stage 21, is what it is for and when it is done. This document is
the order it was done in, what was counted before it began, and each decision as it was taken. What a
decision refused is `stage21_refused.md`.

It is the library's half of a stage of a host's plan, and is built here as *From here, a
host's* of `plan.md` says such work is: as a stage of this plan with its own number.

## What was ruled before it began

A host that is being written put four questions, and Enkrid ruled each on 2026-10-08:

    what a host is handed    units: the library, and each backend as a unit of its own
    who makes them           the library, by a step of its own build, held by a case
    how a host takes them    its build fetches them from where the library publishes them, at
                             a commit and by the hash of each file
    when the platforms run   for `main` only, and that run publishes what a host is handed

## What there was (measured on `c317353`, 2026-10-08)

    what the build makes   one folder for a target, about two hundred files: the shared
                           library (333 KB on Windows x86-64), the static one (3.1 MB), the
                           backends in a folder of their own - the native one (193 KB), the
                           wasm one, wasmtime's shared library (37 MB) and three that are the
                           tests' - and the programs and modules of the tests
    the headers            in the source tree. `bridge.h` is the one a host includes
    no step                sets apart what a host takes: `build.py` has `build` and `test`
    its own host           the program of the tests, compiled against `include/` and linked
                           with the shared library, then with the static one, inside the tree
    the workflows          three, for `dev` and pull requests since the day before. Each job
                           runs every case, then every case six at once ten times over. Five
                           jobs: Windows x86-64 in 39 minutes, Linux x86-64 in 27 and arm64 in
                           16, macOS x86-64 in 21 and arm64 in 8: about 430 counted minutes
    Windows arm64          compiled and linked from x86-64, and not run
    published              nothing: no release and no tag, and the repository is private

## The order

    1  the stage        this document, and stage 21 in `plan.md` with a done-when
    2  the decisions    nine, one at a time, before a file changes
    3  the step         the build writes the units for a target, with their statement
    4  what holds it    a case fails on a unit that is not what the document says; and the
                        program of the tests is built again from the units alone
    5  the document     `host_api.md` says what a host is handed, and where
    6  the workflows    for `main` only, and a run publishes the units of each platform
    7  the first run    the merge to `main`: what was published is fetched back and its
                        hashes checked
    8  close            put to Enkrid by the done-when

## The decisions

### What it is called

**A package.** Enkrid ruled it, as recommended. What a host takes is a package: the library's
package, a backend's package. The step of the build is `python build.py package`, and the
whole has no name of its own: it is the packages of a target. It is the ordinary word for
built files set apart for somebody else to take, and `readme.md`, *Words*, gains it.

### What the library's package holds

**The shared library, the static one, `bridge.h`, `license`, and its statement.** Enkrid ruled
it, as recommended. It is what a host links and what the licence asks to travel with it; the
statement says which commit it is, and so where its source is. The debug symbols are not in
it, and no document of the tree is: `licence.md` cites other documents, which outside the tree
are not there.

### What a backend's package holds, and the SDK's

**Two packages for a backend: its own, and its SDK's.** Enkrid ruled it, as recommended.

    a backend's package    its library, and what has to sit beside it: for the wasm backend,
                           wasmtime's library and wasmtime's licence. What a host needs to
                           run modules on that backend
    an SDK's package       `bridge_abi.h`, `bridge_schema.h` and that backend's `sdk/`. What
                           somebody needs to write a module for it

Two audiences, and each takes what it uses: a host that only runs other people's modules
never opens a header of the module seam. With the library's, that is five packages for a
target. The three backends the tests build for themselves are in none.

### The statement a package carries

**A text file, `package.txt`: the package's name, the commit and the target, then for each
file its SHA-256 and its name.** Enkrid ruled it, as recommended. It is read by eye and by a
few lines of any language, so a host's build checks what it fetched without a reader for a
format. It is not called a manifest: that word is a module's - `module.md`, *The manifest*.

### Where the packages are published

**A release for each published commit of `main`, named for the commit and marked as not a
release proper.** Enkrid ruled it, as recommended. Its tag is `main-` and the first twelve
characters of the commit; it holds one archive for each package of each target. It stays, it
is fetched by a plain address made from the commit, and its name claims no version, of which
Bridge counts none.

What it costs: a tag and a release for every merge to `main`, which pile up; and about 190 MB
uploaded each time, since the wasm backend's package holds wasmtime's library for each target.

### Which targets are published

**Six: the five that are run, and Windows arm64, which says in its statement that it was not.**
Enkrid ruled it, over a recommendation to publish only what had run. That target is built
from x86-64, as it is at every close, by the job that runs Windows x86-64. So `package.txt` has
one line more than was ruled for it: whether the cases passed on that target in the run that
published it. A host on Windows arm64 has something to take, and is told what it is.

### What a run does when a platform fails

**Each platform publishes its own.** Enkrid ruled it, over a recommendation of one workflow
whose last job publishes only when all five have passed. The three workflows stay three, one
for a system, and a job that passes attaches its packages to the release of its commit. A
failure on one system does not keep a host on another from a package. What it costs: a release
may lack some targets, and stays so, and the first job to finish is the one that makes it.

### The run under load

**One round, of six copies at once.** Enkrid ruled it, as recommended. The run of 2026-10-08
was measured step by step: every case once took one to four minutes of a job, and every case
six at once, ten times over, took six to twenty-five, which is nine tenths of it. One round
still runs every case under load on every platform, which is what once found a failure that a
case alone never showed, and by those times a run is about 90 counted minutes where it was
430. A failure that shows once in ten rounds is seen a tenth as often.

### What holds it

**A case on what each package holds, and a small proof built from the packages alone.** Enkrid
ruled it, as recommended. The done-when had said the program of the tests would be built from
the packages. It cannot be: that program includes `bridge_abi.h`, which is the module seam's
and is in an SDK's package, and its cases load the modules and the three backends that exist
for the tests, which are in no package. Nothing here had ever been built as a host outside the
tree is.

So the proof is a program of its own, small: it includes only what the library's package
holds and links that library, shared and then static; it loads, through each backend's
package, one module that was compiled against that backend's SDK package and nothing else;
and it creates a deck, runs a phase and destroys it. The build points it at the folders of the
packages and nowhere else.

The done-when is reworded: a host, and a module for each backend, are built from the packages
alone and run.

## What was built

    `python build.py package`   builds, runs every case where the target is the machine's
                                own, writes the five packages with their statements, and
                                makes an archive of each. Every build writes the packages,
                                saying `run no`; this is what says `run yes`
    `tools/package.py`          the case `package`: each package holds what `build.py` says a
                                package holds, the document names it, every hash is the
                                file's, and a header or a licence is the tree's
    `tests/packaged/host.c`     a host built from the library's package alone, linked with
                                the shared library and again with the static one
    `tests/modules/packaged/`   a native module and a wasm one, each built from its SDK's
                                package alone
    two cases                   that host loads both modules through the two backends'
                                packages, runs three passes and destroys the deck
    `host_api.md`               *What a host is handed*
    the three workflows         for `main`; the cases once, then six at once, once; the
                                packages attached to the release of the commit; and the
                                Windows job builds and attaches Windows arm64's, unrun

## What was found on the way

**A host built from the library's package could not state a module.** `bridge_module_config_t`
has a key for the number of the module seam, and that number is in `bridge_abi.h`, which is
the module seam's header and is in an SDK's package. The program of the tests includes that
header, and so had never met this. It is as the design has it and not a defect of the seam: the
number is the module's to state, in its manifest, and a host passes on what it read. What is
missing is what reads a manifest for a host - `module.md`, *The parser of a manifest* - and
until it is built a host is told the number by whoever built the module. `host_api.md` says so,
and the proof is told it by the build, out of the SDK's package.

**The library said what was wrong, the first time it refused that host.** The host had stated
that its modules spawn nothing, and they spawn one entity each: "it stated that it spawns
nothing". No document had to be opened.

**The two proof modules are with the other modules of the tests**, and not beside the host
that loads them: the coding style lets a module keep its state in the variables of its file
only under `tests/modules`, and no exception was added for a second place.

## The first run for `main`

On 2026-10-08, of the commit `6eabdde`, at Enkrid's word:

    macOS      arm64, x86-64     passed, in 2 and 5 minutes
    Linux      arm64, x86-64     passed, in 3 and 3 minutes
    Windows    x86-64            passed, in 14 minutes, the packages of Windows arm64 built
                                 in it

It made the release `main-6eabdde1bd62`, which holds 30 archives: the five packages of each
of six targets. Five of the six say `run yes`, and Windows arm64 says `run no`. It was the
first time the step that publishes had run, and it ran as it was written.

Five packages were fetched back by that commit, from five targets, and each was unpacked and
checked: its statement names that commit and its target, and every file is the one its hash
is of.

A run was about 430 counted minutes the day before, and this one about 105: 6 of Linux, 7 of
macOS counted ten times, 14 of Windows counted twice.

## The close

Put to Enkrid on 2026-10-08, who ruled it done.

    one command writes, for a target, the library's       met: `python build.py package`,
    package and, for each backend, its own and its SDK's   on six targets
    each holding what `host_api.md` says, and a           met: the case `package`. Shown
    statement of the commit and of the hash of each       failing on a file removed, one
    file, and a case fails on a package that differs      added, one changed, and a statement
                                                          changed
    a host, and a module for each backend, are built      met: two cases, with the shared
    from the packages alone and run                       library and with the static one,
                                                          on the five platforms that are run
    a run for `main` publishes the packages of each       met: the run above
    platform
    from which one is fetched by its commit and its       met: five fetched and checked
    hashes hold

Every case passes: 484, optimised and debug, alone and six at once, where the code is
written, and on the five platforms by the run.

**What is not shown.** That a host which is not this tree's own builds from what was
published: that is a host's to show. And that the release can be fetched by a build that is
not its author's: the repository is private, and a host's build has to prove who it is.

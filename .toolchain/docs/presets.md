# .toolchain/presets

A preset holds the files of the configuration for one kind of project, for example a module of a backend.
Only the repository `mc23t` has `.toolchain/presets`. A project copies the files of its preset into `.toolchain/config`.
The tools then read them as the other files of the configuration. `.toolchain/docs/project.toml.md` tells how the files merge.

A preset can also hold a guide. A project copies such a guide over the guide of the same name in `docs/contributing`.

## native_module

`native_module` is the preset of a native module that runs on Bridge. Such a module is a shared library that BB-Native loads.
Bridge-Host reads the manifest of the module, and its program `{bridge_host}/bridge-host{exe}` runs the cases of the module.
On Linux and on macOS, the zip of Bridge-Host records the execute bits of that program, and `fetch` keeps them.

| File | Content |
| --- | --- |
| `native_sources.toml` | The sources `bridge`, `bb_native` and `bridge_host`: the releases `v0.8.0` of Bridge, `v0.5.1` of BB-Native and `host-v0.8.0` of Bridge-Host. The zip of Bridge-Host holds a folder for each target, so its source takes `top`. |
| `native_targets.toml` | For each target, the section of the manifest, and the sha256 of each release. |
| `native_kinds.toml` | The variable `native_flags`, and the kind `native_module`. |
| `native_checks.toml` | `[check.makes]` with the 6 macros of `bridge_schema.h` of Bridge. Each makes a type, or its descriptor `<type>_desc`, from its first argument. |

The kind `native_module` is the kind `library`, with its step `each`, and with these changes:

- It includes the headers of the two sources.
- It hides each name that the module does not export, with `-fvisibility=hidden`.
- It gives the macro `MANIFEST`: the items of `manifest`, with a null character after each item.
- It gives the macro `MANIFEST_SECTION`: the section of the target.
- After the build, it stops if the module does not hold the manifest.

The folders of the headers, the visibility and the two macros are the items of the variable `native_flags`.
The `flags` of the kind name that variable, so each syntax tree of a changed copy gets them too: `.toolchain/docs/rewrite.py.md`.
A kind of a project that makes a native module with steps of its own names `{native_flags}` too. Thus the flags stay in one place.

The folder `into` of each source is `build/third_party/<source>/<target>`, a path from the folder of the project.
Thus a build with `--out`, for example a case that builds a release, finds the sources that `fetch` got.

A source of the module puts the manifest into the section with this line:

```c
[[gnu::section(MANIFEST_SECTION), gnu::used, gnu::retain]] static const char WHO[] = MANIFEST;
```

## wasm_module

`wasm_module` is the preset of a wasm module that runs on Bridge. BB-Wasm loads such a module, and runs it in its sandbox.

| File | Content |
| --- | --- |
| `wasm_sources.toml` | The sources `binaryen`, the release `version_133` of binaryen, and `bb_wasm`, the release `v0.7.0` of BB-Wasm. |
| `wasm_targets.toml` | For each target, the archive of binaryen, and the sha256 of each release. |
| `wasm_kinds.toml` | The variables `wasm_simd`, `wasm_compile` and `wasm_features`, the programs of binaryen in `[tools]`, and the kinds `wasm_text`, `wasm_module` and `wasm_library`. |

The headers of BB-Wasm include `bridge_abi.h` of Bridge, so a project of wasm modules also takes the source `bridge` of `native_module`.
An artifact of the kind `wasm_module` names the accessors in `link`, for example `{bb_wasm}/acc.wasm`, and the headers in `include`: `{bridge}/include` and `{bb_wasm}/include`.

The kind `wasm_module` makes a module in 6 steps:

1. zig compiles the sources for `wasm32-freestanding`, with no library of C, into `<name>.raw.wasm`. It links the files of `archives`.
2. `.toolchain/tools/wasm_guard.py` checks the imports of that module: `.toolchain/docs/wasm_guard.py.md`.
3. `wasm-opt` optimizes the code of the module with `-O3`.
4. `wasm-merge` merges the accessors of `link` into the module, under the name `acc`.
5. `wasm-opt` removes the exports of the accessors and each part that nothing uses. It runs no other pass, so each accessor keeps the body that BB-Wasm checks at load.
6. It writes the items of `manifest` into the custom section `bridge:manifest`, with a zero byte after each item. Bridge-Host reads the manifest there.

`wasm_compile` holds the flags of the target and of the language of step 1: `-target wasm32-freestanding`, `{language}`, `-Wno-missing-field-initializers`, `-O2`, `{debug_info}`, `{float}`, `{wasm_simd}` and `-nostdlib`.
With `{debug_info}`, the module or library of a release holds no debug information, so no folder of the machine that built it.
The step 1 and the command `dump` of the kind both use it. Thus the syntax tree of a changed copy sees the code as the module compiles it, also the code under `__wasm__`.

In the build of the tree of a run, the key `probed` of the kind adds 2 flags to step 1: `.toolchain/docs/toolchain.toml.md`, *[kinds.\<kind\>]*.

- `{tree}/c23b_area.c` holds the area of the probes: `.toolchain/docs/rewrite.py.md`, *The area*.
- `-Wl,--export=c23b_outcomes` exports the address of the area as a global, for the host: `.toolchain/docs/outcomes.h.md`, *A host of wasm modules*.

A normal build gets neither, so a module of a normal build has no export of the probes. zig refuses `--export-if-defined`, so the area comes with the flag that exports it.

The kind `wasm_library` makes a static library `<name>.a` for `wasm32-freestanding`, which a wasm module links.
Its step `each` compiles each source with `wasm_compile`, `{flags}`, `{defines}` and `{include}`, as step 1 of `wasm_module` does.
Then `zig build-lib -static` makes the library from the objects, with `{strip}`. The library has its own command `dump`, the same as the one of `wasm_module`.
A module names the library in `archives`, for example `archives = ["{out}/convex.a"]`.

With `{float}`, the code of a library or of a module joins no product and no sum. Each command has `{flags}` after `wasm_compile`, so an artifact can override that: `.toolchain/docs/toolchain.toml.md`, *The rules of floating point*.

The kind `wasm_text` makes a module from wasm text with `wasm-as`. It holds no accessor, so it is for a test or for the accessors themselves.

## engine

`engine` is the preset of a project of the engine. Such a project does its work in rounds, and its parts are modules.

| File | Content |
| --- | --- |
| `needs.md` | The guide `docs/contributing/needs.md` for such a project. The needs Fast, Scalable, Modular and Composable have the meaning that they have for an engine. |

# .toolchain/presets

A preset holds the files of the configuration for one kind of project, for example a module of a backend.
Only the repository `my_c23_template` has `.toolchain/presets`. A project copies the files of its preset into `.toolchain/config`.
The tools then read them as the other files of the configuration. `.toolchain/docs/project.toml.md` tells how the files merge.

## native_module

`native_module` is the preset of a native module of Bridge. Such a module is a shared library that BB-Native loads.
Bridge-Host reads the manifest of the module.

| File | Content |
| --- | --- |
| `native_sources.toml` | The sources `bridge` and `bb_native`: the release `v0.1.2` of Bridge and the release `v0.1.1` of BB-Native. |
| `native_targets.toml` | For each target, the section of the manifest, and the sha256 of the two releases. |
| `native_kinds.toml` | The kind `native_module`. |
| `native_checks.toml` | `[check.makes]` with the 6 macros of `bridge_schema.h` of Bridge. Each makes a type, or its descriptor `<type>_desc`, from its first argument. |

The kind `native_module` is the kind `library`, with these changes:

- It includes the headers of the two sources.
- It hides each name that the module does not export, with `-fvisibility=hidden`.
- It gives the macro `MANIFEST`: the items of `manifest`, with a null character after each item.
- It gives the macro `MANIFEST_SECTION`: the section of the target.
- After the build, it stops if the module does not hold the manifest.

The folders of the headers, the visibility and the two macros are the `flags` of the kind.
Thus the syntax tree of `test --branches` and of `test --leaks` gets them too: `.toolchain/docs/rewrite.py.md`.

A source of the module puts the manifest into the section with this line:

```c
[[gnu::section(MANIFEST_SECTION), gnu::used, gnu::retain]] static const char WHO[] = MANIFEST;
```

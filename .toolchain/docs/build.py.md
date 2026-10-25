# .toolchain/tools/build.py

`build.py` builds the artifacts of the project into the scratch folder, and lists them in `package.txt`.
It then composes the deliverable of the picked parts into the folder `dist`.
With `--pack`, it also puts the deliverable into a zip file.
`test.py` uses its functions `build`, `prepared`, `ready` and `made`. `branches.py` uses `artifacts` and `planned`.

## Functions

| Function | Result |
| --- | --- |
| `tools(config, values)` | Finds each program of `[tools]`, and sets its variable. |
| `leb(n)` | Gives a number as an unsigned LEB128, the format of the numbers of WebAssembly. |
| `do(step, values, unstamped)` | Does one step of a kind. |
| `put(path, data)` | Writes a file only if it is not there or is different. |
| `moved(path, tree)` | Gives the path in `tree` for a path of `src` or of `include`, and else the path. With no tree, it gives the path. |
| `planned(config, values, artifact, tree)` | Gives the kind and the variables of one artifact, as steps 1 to 4 of *Artifacts* tell. |
| `built(config, values, artifact, tree)` | Builds one artifact. It gives the name of the artifact and its files. |
| `packaged(config, values, made)` | Writes `package.txt`. |
| `leaked(values, kit)` | Stops if a file of the deliverable `kit` holds the folder of the project or of the build, as *The zip* tells. |
| `packed(config, values, kit)` | Writes the zip of the deliverable `kit`, after `leaked`. Then it writes the path of the zip and the names in it. |
| `artifacts(config)` | Gives each module of `[[modules]]` over the defaults of `[module]`, then each artifact of `[[artifact]]`, and after each one its variants. |
| `path_defines(config)` | Gives the `*_PATH` macros of the test libraries, for the variable `paths`. |
| `archives(config, values)` | Gives the path of each shipped archive, from the output folder. |
| `prove(config, values, pick)` | Proves the package, as *The package proof* tells. It does nothing if `[package]` has no `proof`, or if `pick` leaves out the base. |
| `composed(config, values, pick)` | Composes the deliverable of the parts of `pick`, as *The deliverable* tells. With no `pick`, it composes every part. It gives the files of the deliverable. |
| `prepared(args, here)` | Gives the configuration and the variables, as steps 1 to 8 of *build* tell. It sets the variable `paths` with `path_defines`. |
| `ready(args, here)` | Does the steps 1 to 10 of *build*, and gives the configuration and the variables. |
| `made(config, values, tree, part, tests)` | Builds each artifact and writes `package.txt`: step 11 of *build*. It gives the name and the files of each artifact. With a list of parts, it builds the base and the artifacts of those parts. With `tests` false, it skips an artifact with `test = true`. |
| `build(args, here, tests)` | Does all the steps of a build. With `here`, it stops if the target does not run here. With `tests` false, it builds no test artifact. |
| `main()` | Runs the commands `build` and `pack`. It builds only the shippable artifacts, composes the deliverable, and after a `pack` makes the zip and proves the package. |

## build

`build` does these steps:

1. It reads the configuration.
2. It takes `--target`, or the target of this machine.
3. It stops if no target runs here and `--target` is not given: `no target runs on <system> <machine>: name one`.
4. It stops if the configuration does not have the target: `no target <target>; the targets: <targets>`.
5. With `here`, it stops if the target is not the target of this machine.
6. It sets the caches of zig to `[build] cache` of `HERE`.
7. It stops if the pinned zig is not there: `zig <version> is neither fetched nor on the path`.
8. It makes the variables. `prepared` does the steps 1 to 8.
9. It checks that each source is fetched.
10. It finds the tools, and takes the dependencies.
11. It builds each artifact, and writes `package.txt`.

`--part <name>` names a part to build. The option repeats, for example `--part base --part wasm`.
Step 11 builds the base and the artifacts of each named part. The base is the untagged artifacts.
It builds the base for every part, so the links of each part resolve.

Without `--part`, step 11 builds every artifact.
`.toolchain/docs/project.toml.md`, *[[artifact]]*, tells the key `part`.

The command `build` and the command `pack` build only the shippable artifacts. Step 11 then skips an artifact with `test = true`, which only `test` builds.

The caches of zig are the variables `ZIG_GLOBAL_CACHE_DIR` and `ZIG_LOCAL_CACHE_DIR` of the environment.
The commands of the steps and of the cases get them too.
When the cache is in the folder of the project, each variable is a path from that folder, since each step runs there.
Thus an archive that zig makes names its members with no folder of the machine. Else each variable is a whole path.
`tools` stops if a program is not there: `the tool <name> is not at <path>`.

## main

`main` builds, then writes the path of each artifact.
It then composes the deliverable of the picked parts with `composed`.
With `--pack`, it writes the zip with `packed`, and proves the package with `prove`.

## Artifacts

`built` does these steps for an artifact. `planned` does the steps 1 to 4:

1. If the artifact gives a `target`, it uses the keys of that target and its name as `{target}`.
2. It replaces the variables of the name, and finds the sources. In `sources`, an item that is only a list variable gives the patterns of that list.
3. It stops if no file matches the sources: `no file matches the sources of <name>`.
4. It sets `{output}`, `{work}`, `{sources}`, `{defines}`, `{include}`, `{flags}`, `{link}` and `{manifest}`.
5. It makes the folder of the output, and the folder `obj`.
6. If a file of `link` is not there, it does the `unlinked` steps of the kind.
7. Else, it does the `steps` of the kind, and copies the `beside` files into the folder of the output.

With a tree, step 4 changes each path that is in `src` or in `include`, for the sources and the folders of `include`.
It gives the copy of the path in the tree.
`test --branches` builds so, from the copy with the probes: `.toolchain/docs/branches.py.md`.

An artifact with a `target` stays in the output folder of the build.
Thus a build for one target can make a file for a different target, for example a library for each system.

A kind with no `unlinked` steps stops `built` if a file of `link` is not there.
The message is `<file> is not there, and <name> is linked with it`.
An artifact that is not linked gives no file. `built` writes `compiled, not linked: <name>: <file> is not there` for it.

## Modules

`[[modules]]` of `project.toml` declares the test modules of the folder `built`.
Each module is an artifact over the defaults of `[module]`. A row states only what differs: a `name`, and `sources` or `defines` when they are not the defaults.
A row can carry `variants`, like an artifact.

`path_defines` makes the variable `paths`: a `*_PATH` macro for each test library, and `BUILT_DIR` and `BUILT_SUFFIX`.
A built module gets a macro only with `path = true`; a backend always gets one.
The macro is the name in upper case, without a `bridge_` prefix, with `-` as `_`, and `_PATH` at the end.
Its value is the output path of the artifact. An artifact's `defines` splices `{paths}` to get every macro.

## Steps

`do` does the actions of a step in this order: `run`, `section`, `contains`, `copy`, `rename`.
A step with a key that `KEYS['step']` does not have stops the tool.
`copy` and `rename` take their paths from the root.

A `section` step adds a custom section of WebAssembly to the end of the output.
The section is the byte 0, the size of the content, then the content.
The content is the size of the name, the name, then the text.
Each size is an unsigned LEB128:

```pseudo
FUNCTION leb(n : INTEGER) RETURNS STRING
   DECLARE out : STRING
   DECLARE low : INTEGER
   out <- ""
   REPEAT
      low <- n MOD 128
      n <- n DIV 128
      IF n > 0 THEN
         low <- low + 128
      ENDIF
      out <- out & CHR(low)
   UNTIL n = 0
   RETURN out
ENDFUNCTION
```

A `contains` step stops the tool if the output does not hold the text: `<output> does not hold <text>`.

## package.txt

`packaged` writes `<out>/package.txt`:

```text
package <project>
target <target>
<digest>  <file>
```

`<project>` is the name that `shared.project_name` gives.
It writes a line `<digest>  <file>` for each file that the build made, one time for each file, but not for an artifact with `test = true`.
The path of the file is from the output folder of the target.

## The deliverable

The build makes each artifact in the scratch folder `out`. A built binary is not a deliverable.
`composed` gathers the deliverable of the picked parts into the folder `dist`, beside `scratch`.

For each part of `pick`, `composed` copies into `dist`:

1. the output of each shipped artifact of the part, and its `beside` files, at the top of `dist`,
2. each header of the part, from `[compose.<part>] headers`, at its path from the root,
3. each asset of the part, from `[compose.<part>] assets`, at the top of `dist`.

It also copies `license`, and `license` of each folder of `third_party`, at their paths from the root.
With no `pick`, it composes every part. A part with no header, for example a host, composes only its binaries.
`composed` writes the path of each file of the deliverable.
`.toolchain/docs/project.toml.md`, *[compose]*, tells the keys `headers` and `assets`.

## The zip

`packed` writes one file, `<build>/<project>-<target>.zip`, in the output folder and not in the folder of the target.
It puts each file of the deliverable `kit` into the zip, at its path from `dist`.
So the zip is the deliverable that `composed` made, and `pack --part` zips the parts that it picks.
`packed` writes the path of the zip, then the name of each file in the zip.

Before it writes the zip, `packed` reads each file of the deliverable.
It stops if a file holds the folder of the project or the folder of the build, with `/` or with `\`: `<file> holds <path>, a path of the machine that built it`.
Thus a package does not depend on the machine that built it, for example a library that names another library by its path.

## The package proof

`[package] proof` names a source that uses only the package.
After a `pack`, `prove` reads it, so a project proves that its package is enough to build against.
`prove` does nothing when `--part` leaves out the base, since the proof links the base archives.

`prove` extracts the zip into `<out>/proof`, so nothing of the source tree is there.
It builds the proof source as a program, with the headers of the package and each shipped archive, in `<out>/proof/host`.
It puts the folder `include` and each sub-folder of `include` on the include path. So the proof finds a header in any folder of the package.
It runs the program if the target runs here, and `shared.run` stops the tool if the program does not end with 0.
It then writes `package proof: <project> ran`, or `package proof: <project> built` when the target does not run here.

`archives` gives the shipped archives that the proof links.
`[package] link` names more files that the proof links, after the shipped archives.
Such a file is not in the package, for example an archive of a different project that `[[source]]` fetches.
`prove` expands each name with the variables of the build, before it gives the proof its own `out`.

`[package] include` names more folders for the include path of the proof, after the folders of the package.
Such a folder holds headers that the headers of the package include, for example the headers of a different project.
`prove` expands each folder in the same way.

## Limits

No case runs these branches.

- A machine on which no target runs. Each machine of the cases runs a target.
- A machine with no pinned zig. Each case uses the zig of the project that runs it.
- A proof built for a target that does not run here. Each case packs for the target of its machine.

# .toolchain/tools/build.py

`build.py` builds the artifacts of the project, and lists them in `package.txt`.
With `--pack`, it also puts the package into a zip file.
`test.py` uses its function `build`.

## Functions

| Function | Result |
| --- | --- |
| `tools(config, values)` | Finds each program of `[tools]`, and sets its variable. |
| `leb(n)` | Gives a number as an unsigned LEB128, the format of the numbers of WebAssembly. |
| `do(step, values, unstamped)` | Does one step of a kind. |
| `put(path, data)` | Writes a file only if it is not there or is different. |
| `built(config, values, artifact)` | Builds one artifact. It gives the name of the artifact and its files. |
| `packaged(config, values, made)` | Writes `package.txt`. |
| `packed(config, values)` | Writes the zip of the package. Then it writes the path of the zip and the names in it. |
| `artifacts(config)` | Gives each artifact, and after it each of its variants. |
| `prepared(args, here)` | Gives the configuration and the variables, as steps 1 to 8 of *build* tell. |
| `build(args, here)` | Does all the steps of a build. With `here`, it stops if the target does not run here. |
| `main()` | Runs the commands `build` and `pack`. |

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

The caches of zig are the variables `ZIG_GLOBAL_CACHE_DIR` and `ZIG_LOCAL_CACHE_DIR` of the environment.
The commands of the steps and of the cases get them too.
`tools` stops if a program is not there: `the tool <name> is not at <path>`.

## main

`main` builds, then writes the path of each artifact.
With `--pack`, it then writes the zip with `packed`.

## Artifacts

`built` does these steps for an artifact:

1. If the artifact gives a `target`, it uses the keys of that target and its name as `{target}`.
2. It replaces the variables of the name, and finds the sources.
3. It stops if no file matches the sources: `no file matches the sources of <name>`.
4. It sets `{output}`, `{work}`, `{sources}`, `{defines}`, `{include}`, `{flags}`, `{link}` and `{manifest}`.
5. It makes the folder of the output, and the folder `obj`.
6. If a file of `link` is not there, it does the `unlinked` steps of the kind.
7. Else, it does the `steps` of the kind, and copies the `beside` files into the folder of the output.

An artifact with a `target` stays in the output folder of the build.
Thus a build for one target can make a file for a different target, for example a library for each system.

A kind with no `unlinked` steps stops `built` if a file of `link` is not there.
The message is `<file> is not there, and <name> is linked with it`.
An artifact that is not linked gives no file. `built` writes `compiled, not linked: <name>: <file> is not there` for it.

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
It writes a line `<digest>  <file>` for each file that the build made, one time for each file.
The path of the file is from the output folder of the target.

## The zip

`packed` writes one file, `<build>/<project>-<target>.zip`, in the output folder and not in the folder of the target.
It puts these files into the zip:

1. `package.txt`, and each file that `package.txt` lists, at its path from the output folder of the target.
2. Each header of `include`, at its path from the root.
3. `license`, and `license` of each folder of `third_party`, at their paths from the root.

`packed` writes the path of the zip, then the name of each file in the zip.

## Limits

No case runs these branches.

- A machine on which no target runs. Each machine of the cases runs a target.
- A machine with no pinned zig. Each case uses the zig of the project that runs it.

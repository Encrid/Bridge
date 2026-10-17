# .toolchain/tools/fetch.py

`fetch.py` gets zig and the sources of the project.
Then it takes the dependencies with `take.py`.
`build.py` uses the functions `zig_found` and `sources` of `fetch.py`.

## Functions

| Function | Result |
| --- | --- |
| `zig_found(zig)` | Gives the path of the pinned zig, or nothing. |
| `fetched(url, digest, folder)` | Downloads an archive into an empty folder, checks its digest, and unpacks it. |
| `fetch_zig(zig, home)` | Downloads the pinned zig for the target of this machine. |
| `hunks(part)` | Gives each hunk of the part of a patch: its first line, its old lines and its new lines. |
| `patched(tree, patch)` | Applies a patch to the files of a folder. |
| `sources(config, values, fetching)` | Fetches each source, or checks that each source is fetched. |
| `main()` | Runs the command `fetch`. |

## main

`main` does these steps:

1. It reads the options and the configuration.
2. It stops if no target runs on this machine: `fetch runs on a machine of a target`.
3. If the pinned zig is not there, it downloads zig. Then it writes the path of zig.
4. It makes the variables, with the target of this machine.
5. It fetches each source.
6. It takes the dependencies with `take.py`.

## zig

`zig_found` looks for `zig` or `zig.exe` in a folder of `[zig] folder`, in `HERE`.
If it finds no zig there, it uses the zig on the path.
It gives the zig only if `zig version` writes the value of `[zig] version`.

`fetch_zig` makes the address of the archive from `[zig] url` and `[zig.archives.<home>]`.
It stops if `[zig.archives]` has no archive for the target of this machine: `zig is fetched on a machine that runs a target`.
It stops if the archive does not hold the pinned zig: `the archive holds no zig <version>`.

## Archives

`fetched` does these steps:

1. It removes the folder, and makes it again.
2. It downloads the archive into the folder.
3. If the archive has a different digest, it removes it and stops: `<archive> is not what the maker publishes`.
4. It unpacks the archive, and removes the archive.

## Patches

A patch has a part for each file. A part starts with `diff --git`.
`patched` reads the name of the file from the line `+++ b/<file>`.

A hunk starts with `@@ -<line>,<count> +<line>,<count> @@`.
Its old lines are the lines of context and the lines that start with `-`.
Its new lines are the lines of context and the lines that start with `+`.
A line that starts with `\` is not a line of the file.

`patched` applies the hunks of a part in their order. The shift starts at 0.

1. The tool stops if the old lines are not at the line of the hunk, plus the shift.
2. The new lines replace the old lines.
3. The shift grows by the number of new lines, less the number of old lines.

The message is `<patch> does not apply to <file>`.
`patched` reads a patch with the line ends of Windows or of Linux.

## Sources

The stamp of a source is the digest of its archive, then the digest of each patch, with a space between them.
`sources` does these steps for each source:

1. It sets the variable `{<name>}` to the `top` folder of the source.
2. If `fetched.txt` of the `into` folder holds the stamp, it goes to the next source.
3. If it does not fetch, it stops: `<name> is not fetched as project.toml states: run fetch`.
4. It fetches the archive into the `into` folder.
5. It applies each patch to the `top` folder.
6. It writes the stamp into `fetched.txt`.

If the address has no `://`, it is a file of the project, and `sources` makes a `file:` address from it.

## Limits

No case runs these branches. The workflows run some of them.

- The download of zig, and its two refusals. Each runner of the workflows downloads zig with `fetch`.
- A source with an address on the network. A case does not use the network.
- A machine that is no target. The machines of the cases are all targets.

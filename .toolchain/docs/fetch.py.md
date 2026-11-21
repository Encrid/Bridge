# .toolchain/tools/fetch.py

`fetch.py` gets zig and the sources of the project.
Then it takes the dependencies with `take.py`.
`build.py` uses the functions `zig_found` and `sources` of `fetch.py`.

## Functions

| Function | Result |
| --- | --- |
| `zig_found(zig)` | Gives the path of the pinned zig, or nothing. |
| `fetched(url, digest, folder, fetch)` | Gets an archive with `kept`, and unpacks it into an empty folder. `fetch` is the table `[fetch]` of `toolchain.toml`. |
| `kept(url, digest, folder, timeout)` | Gives the path of the archive in the shared folder `folder`. It downloads the archive first if it is not there, as *The shared archives* tells. |
| `downloaded(url, part, had, timeout)` | Downloads the bytes of the address after the first `had` bytes, into the file `part`. It tells if the download came to its end. |
| `held(lock)` | Waits until this process holds the lock of the file `lock`. |
| `unpacked(path, folder)` | Unpacks an archive into a folder, as *Archives* tells. |
| `fetch_zig(zig, home)` | Downloads the pinned zig for the target of this machine. |
| `hunks(part)` | Gives each hunk of the part of a patch: its first line, its old lines and its new lines. |
| `patched(tree, patch)` | Applies a patch to the files of a folder. |
| `sources(config, values, fetching)` | Fetches each source, or checks that each source is fetched. |
| `main()` | Runs the command `fetch`. |

## main

`main` does these steps:

1. It reads the options and the configuration. With `--toolchain`, it fetches for each target of the toolchain: `.toolchain/docs/project.toml.md`, *[project]*.
2. It stops if no target of the project runs on this machine: `fetch runs on a machine of a target`.
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

1. It gets the archive with `kept`: *The shared archives*.
2. It removes the folder, and makes it again. If the system cannot remove the folder, `fetched` stops with the error of the system.
3. It unpacks the archive with `unpacked`.

## The shared archives

`[fetch] archives` of `toolchain.toml` names the folder of the archives, from the folder of `project.py`.
Each archive is in the folder of its digest, in that folder. Thus each project and each case of this machine uses the same archive, and downloads it 1 time.

`kept` does these steps:

1. It holds the lock of the folder of the digest. Thus 2 processes do not download 1 archive at the same time. The system frees the lock when a process ends, also when a process is killed.
2. If the archive is there, it gives it.
3. Else it downloads the archive into the file `<archive>.part`. If that file holds bytes already, it asks the server only for the next bytes. Thus a killed download goes on. If the server sends all the bytes, it writes the file again.
4. If the download stops before its end, it goes on from the new end. If the download gives no new byte, it stops: `<archive> stops at <count> bytes: run fetch again`.
5. If the file has a different digest, it removes the file and stops: `<archive> is not what the maker publishes`.
6. It renames the file to the archive. Thus the folder holds an archive only after its digest is checked.

A read that waits for more than `[fetch] timeout` seconds stops the download, so a download that hangs goes on, or stops.

After `fetched`, `sources` stops if the source states a `top` whose folder the archive does not have: `<source>: the archive has no folder <top>`.
Thus a release with no folder for a target stops the fetch, and not the build later.

`unpacked` gives an archive that is not a zip to `shutil.unpack_archive` of Python, which keeps the execute bits of its files.
Python keeps no mode of the files of a zip. Thus `unpacked` unpacks each file of a zip, then adds the execute bits that the zip records for it.
So a program of a zip can run on Linux and on macOS. Windows has no execute bit, so the bits change nothing there.

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

`top` can hold variables, for example `{target}`. Then one archive, with one digest, holds the files of each target in a folder of its own.
The command `fetch` fetches it one time. Each build sets `{<name>}` to the folder of its own target.
`fetch` applies the patches only to the `top` folder of the target of this machine.

## Limits

No case runs these branches. The workflows run some of them.

- The download of zig, and its two refusals. Each runner of the workflows downloads zig with `fetch`.
- A source with an address on the network, a download that goes on, and a download that gives no new byte. A case of `fetch` does not use the network.
- A machine that is no target. The machines of the cases are all targets.

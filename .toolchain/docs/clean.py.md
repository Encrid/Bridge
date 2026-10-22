# .toolchain/tools/clean.py

`clean.py` removes the files that the toolchain made in the output folder.
The commands `clean` and `distclean` of `project.py` run it.

```sh
python .toolchain/tools/clean.py [--root <folder>] [--out <folder>] [--all]
```

## The output folder

The output folder is the variable `build` of `shared.variables`.
If the output folder is not there, `clean.py` writes `nothing to clean: <folder> is not there`.

## clean

Without `--all`, `clean.py` removes each file and each folder of the output folder, but not these folders:

- the folder of zig, `[zig] folder` in the folder of `project.py`,
- the folder `into` of each source of `[[source]]`.

Thus the next build does not download zig and the sources again.
`clean.py` keeps a folder that holds one of these folders, and removes its other files.
`clean.py` removes a read-only file too, for example a git object in the scratch of the tests. It clears the read-only bit first.

## distclean

With `--all`, `clean.py` removes the full output folder.
Then the project has only the files that git tracks, as after a new copy.

After each of the two actions, `clean.py` writes `cleaned <folder>`.

## Functions

| Function | Result |
| --- | --- |
| `writable(func, path, _)` | Clears the read-only bit of a path, then does the action again. It lets `scrub` remove a read-only file. |
| `scrub(path)` | Removes a folder with its files, and removes a read-only file in it with `writable`. |
| `removed(folder, kept)` | Removes each file and each folder of `folder`, but not the folders of `kept` and the folders that hold them. |
| `main()` | Reads the options, finds the output folder and the folders to keep, and removes the rest. |

## Limits

`clean.py` makes the variables of the target of this machine, to find the folders of the sources.
On a machine on which no target runs, it stops: `no target runs here: name one`. No case runs this branch.

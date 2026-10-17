# .toolchain/tools/licence.py

`licence.py` checks the licence of the project, and the notices of the code of other holders.

```sh
python .toolchain/tools/licence.py [--root <folder>]
```

## The checks

1. `license` is the text of the GNU Affero General Public License, version 3, as its authors publish it.
2. No file of the project says that all rights are reserved.
3. Each folder `third_party/<source>` holds the file `license`.
4. `readme.md` names `third_party/<source>/license` for each source.
5. `cla.md` is there, and `readme.md` names it.

For check 1, `licence.py` compares the SHA-256 digest of `license` with the digest of the published text.
The published text is `https://www.gnu.org/licenses/agpl-3.0.txt`: 661 lines and 34523 bytes, with the line ends of Linux.
Its digest is `0d96a4ff68ad6d4b6f1f30f713b18d5184912ba8dd389f86aa7710db079abcb0`.
`.gitattributes` keeps the line ends of Linux on each system. Thus the digest is the same on each system.

For check 2, `licence.py` reads each file of the project as text, but not these files:

- `license`,
- the files of `third_party`, because their code has the notices of its holders,
- each file that `ours` leaves out,
- a file with the byte 0 in its first 4096 bytes, because it is not text.

For checks 4 and 5, `readme.md` names a file if it holds the path of the file in a code span.

## Output

The output follows `.toolchain/docs/shared.py.md`, *The report of a check*.

| Fault | Line |
| --- | --- |
| `license` is not there. | `license is not there` |
| `license` is a different text. | `license is not the published text of the licence` |
| A file says that all rights are reserved. | `<file>:<line>: says that all rights are reserved` |
| A source has no notice. | `third_party/<source>/license is not there` |
| `cla.md` is not there. | `cla.md is not there` |
| `readme.md` does not name a notice. | `readme.md does not name <file>` |

## Functions

| Function | Result |
| --- | --- |
| `faults(root, files)` | Gives the faults of the project in `root`. `files` holds the files of the project. |
| `main()` | Reads the options, and writes the faults. |

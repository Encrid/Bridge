# .toolchain/tests/cases/said.toml

The group `said` checks `.toolchain/tools/said.py`.
Each case writes a small project into `{case}/p` with `files`, and runs `said.py --root {case}/p --out {case}/out`.
In this document, each path of the small project starts with `{case}/p`.

| File | Content |
| --- | --- |
| `{case}/p/project.toml` | `[test] writers` names the writer `say`. |
| `{case}/p/src/p.c` | One call of `say`, on the first line. |
| `{case}/out/{target}/said.txt` | The output of the cases. |
| `{case}/p/readme.md` | The text that quotes the messages. |

| Case | Claim | False variant |
| --- | --- | --- |
| a message that no case wrote | The format is `"the file %s is not there\n"`, and the output has a different text. `said.py` writes `src/p.c:1: no case wrote: the file %s is not there`. | The output has `the file a.txt is not there`. |
| a message that no document quotes | The output has the message, and the readme does not quote it. `said.py` writes `src/p.c:1: no document quotes: the file %s is not there`. | The readme quotes `the file <path> is not there`. |
| a message that a document of docs/extern quotes | `{case}/p/docs/extern/p.md` quotes the message. `said.py` ends with 0. | The document quotes `the file is there`. |
| a format in two strings | The format is `"the file " "%s is gone\n"`. The output has `the file a.txt is there`. `said.py` writes `no case wrote: the file %s is gone`. | The output has `the file a.txt is gone`. |
| a format with few own characters | The format is `"%s\n"`. `said.py` ends with 0. | The format is `"%s is gone\n"`. |
| a function that is not a writer | `{case}/p/src/p.c` calls `puts`, which is not a writer. `said.py` ends with 0. | `[test] writers` names `puts`. |
| no output of the cases | `said.txt` is not there. `said.py` stops: `said.txt is not there: run test first`. | `said.txt` is there. |
| the messages after the cases | `test.py` runs on the small project. Its one case writes `the file a.txt is not there`. Then `test.py` writes `no fault in 1 messages`. | The readme does not quote the message. |

The last case runs `test.py`, and not `said.py`. Its `folders` copy the group `.toolchain/tests/data/said/said.toml` into `{case}/p/tests/cases`.
That group has one case, which writes the message.

# .toolchain/tests/cases/cla.toml

The group `cla` checks `decide` of `.toolchain/tools/cla.py`, through `cla.py --decide`.
The authors are `ann` and `bob`.

| Case | Claim | False variant |
| --- | --- | --- |
| a name of the record | The record holds `bob` and `ann`. `cla.py` writes `nothing, passes: ann has signed cla.md`. | The record holds `bob`. |
| the sentence of the author | `ann` writes the sentence. `cla.py` writes `record, passes: ann signs cla.md`. | `bob` writes the sentence. |
| each other author | `ann` writes `I sign, more or less.`. `cla.py` writes `ask, fails: ann has not signed cla.md`. | `ann` writes the sentence. |
| a decision with too few values | `cla.py --decide ann` stops: `usage: cla.py --decide <author> <signed> [<by> <said>]`. | The record is the second value. |

`.toolchain/docs/cla.py.md` gives the limits of these cases.

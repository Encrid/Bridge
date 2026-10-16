# .toolchain/tests/cases/take.toml

The group `take` checks `.toolchain/tools/take.py`.
Its cases run `fetch.py` on the project in `.toolchain/tests/data/take`, with the output in `{case}/out`.
That project has three dependencies:

- `sample` takes the folder `{sample}`. The default of `sample` is `sample`.
- `made` copies the folder `{made}` into `{build}/made` with its `make`, then takes it. The default of `made` is `sample`.
- `maybe` takes the folder `{maybe}`, and is optional. The default of `maybe` is `nothing`, a folder that is not there.

An `[[agree]]` compares the key `package` of the three dependencies.

| Folder | Content |
| --- | --- |
| `sample` | `greeting.txt` and its list. The list gives the key `package` the value `sample`. |
| `tampered` | A `greeting.txt` that does not have the digest of its list. |
| `listed` | The list of `sample`, without `greeting.txt`. |
| `other` | A different `greeting.txt` and its list. The list gives the key `package` the value `other`. |

| Case | Claim | False variant |
| --- | --- | --- |
| a dependency, taken | The copy of `greeting.txt` holds `hello from a dependency`, and `fetch` writes `taken: sample, package sample`. | `--set sample=tampered`. |
| a dependency that is not what its list states | With `--set sample=tampered`, `fetch` stops: `refused` and `greeting.txt is not what`. | No `--set`. |
| a copy that is there already | With `--set sample=listed` and a right copy of `greeting.txt`, `fetch` passes. | The copy is different, thus `fetch` stops: `greeting.txt is not there`. |
| a dependency that is made first | `fetch` writes `taken: made, package sample`. | `--set made=nothing`, thus `make` fails. |
| an optional dependency that is not there | `fetch` writes `not taken:` and `nothing/package.txt is not there`. | `--set maybe=sample`. |
| a list that is not there | With `--set sample=nothing`, `fetch` stops: `nothing/package.txt is not there`. | No `--set`. |
| dependencies that do not agree | With `--set made=other`, `fetch` stops: `refused: the dependencies do not agree`. | No `--set`. |

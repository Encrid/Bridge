# .toolchain/tests/cases/fetch.toml

The group `fetch` checks `.toolchain/tools/fetch.py`.
Its cases run `fetch.py` or `build.py` on the project in `.toolchain/tests/data/fetch`, with the output in `{case}/out`.
That project has one source, `sample`: the archive `sample.zip` and the patch `.toolchain/tests/data/fetch/third_party/sample/<patch>.patch`.
The variable `patch` is `greeting`, and the variable `digest` is the digest of `sample.zip`.

The archive holds `sample/greeting.txt`. The patch `greeting` changes its line `hello` to `hello, patched`.
The patch `wrong` has a line of context that the file does not have.

| Case | Claim | False variant |
| --- | --- | --- |
| a source, patched | `fetch` patches `greeting.txt`, and the file holds `hello, patched`. | `--set patch=wrong`. |
| a patch that does not apply | With `--set patch=wrong`, `fetch` stops: `wrong.patch does not apply to greeting.txt`. | No `--set`. |
| a source that is not what its maker publishes | With a different digest, `fetch` stops: `sample.zip is not what the maker publishes`. | The digest of `sample.zip`. |
| a source that is not fetched | `build` stops: `sample is not fetched as project.toml states: run fetch`. | `fetch` in place of `build`. |
| a source that is fetched already | `fetched.txt` holds the stamp, thus `fetch` keeps `greeting.txt` as it is. | `fetched.txt` holds a different stamp. |

`.toolchain/docs/fetch.py.md` gives the limits of these cases.

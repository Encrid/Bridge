# .toolchain/tests/cases/clean.toml

The group `clean` checks `.toolchain/tools/clean.py`.
Each case writes a small project into `{case}/p` with `files`.
Its `.toolchain/project.toml` names the source `s`, with the folder `build/third_party/s`.
The first two cases run a short Python program. It runs `clean.py`, then writes each file that is still in `{case}/p`.

| Case | Claim | False variant |
| --- | --- | --- |
| clean keeps the sources | `clean.py` removes `build/x/a.txt` and `build/p-x.zip`, and keeps `build/third_party/s/fetched.txt`. | `clean.py --all`. |
| distclean removes the folder build | `clean.py --all` removes `build/x/a.txt` and `build/third_party/s/fetched.txt`. `.toolchain/project.toml` stays. | No `--all`. |
| nothing to clean | The project has no folder `build`: `clean.py` writes `nothing to clean`. | The project has `build/a.txt`. |

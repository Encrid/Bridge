# .toolchain/tools/workflows.py

`workflows.py` checks that each workflow with a matrix builds each target of the toolchain.
The command `check` runs it.

## The workflows

| Workflow | When it runs | What it does |
| --- | --- | --- |
| `ci.yml` | On each pull request, and on each push to `main`. | On each target, it prepares the runner with `runner.py`, then runs `fetch`, the five commands and `pack`. `docs/contributing/pull_request.md`, *Before you open a pull request*, gives the five commands. |
| `cla.yml` | On each pull request, and on each new comment of a pull request. | It runs `cla`, which asks the author to sign `cla.md`, and sets the status `cla`. |
| `release.yml` | On each push of a tag `v*` or `*-v*`. | It makes the release of the project or of a part, as *The release* tells. |
| `toolchain.yml` | Only in the repository `my_c23_template`: on each pull request, and on each push to `main`. | On each target, it runs `fetch`, `test --toolchain` and `test --toolchain --break`. |

The step `Runner` of `ci.yml` and of `release.yml` gives the runner what the cases need: `.toolchain/docs/runner.py.md`.

## The release

`release.yml` runs on the tag of a release: `docs/contributing/pull_request.md`, *The release*.
It gives the tag to the tools with `--tag`. A tag `<part>-v<major>.<minor>.<patch>` names a part, and a tag `v<major>.<minor>.<patch>` names the project.
`release.yml` does these steps:

1. It makes a draft release, with the name of the tag.
2. On each target of its matrix, it prepares the runner with `runner.py`, then runs `fetch`, `test` and `pack --target <target>`.
3. On each target, it adds the zip of the target to the draft.
4. It runs `join`, which joins the zips of the targets of the part of the tag. For the tag of the project, `join` joins nothing.
5. For the tag of the project, it adds `sha256.txt`, with the sha256 of each zip, to the draft.
6. For the tag of a part, it removes the zips of the targets from the draft. It adds `<name>.zip` and its `sha256.txt` in their place.
7. It publishes the release. The title of the release of a part is `<name> v<major>.<minor>.<patch>`, and it is not the latest release.

Thus the release of a part has 2 files: `<name>.zip`, with a folder for each target, and `sha256.txt`.

A project that uses the release of a project names the zip of each target in a `[[source]]`, with the sha256 that `sha256.txt` gives.
A project that uses the release of a part names `<name>.zip` in one `[[source]]`, with `top = "{target}"`.
`.toolchain/docs/project.toml.md`, *[[source]]*, gives the keys.

## The check

`main` reads each file `.github/workflows/<name>.yml` of the project.
A line `- target: <name>` names a target of the matrix of the workflow.
A workflow with no such line has no matrix, and `main` does not check it.

For a workflow with a matrix, `main` writes these faults:

- `<path>: the matrix does not build <target>`, for each target of `[targets]` that the matrix does not name.
- `<path>: <name> is not a target of the toolchain`, for each name that `[targets]` does not have.

Thus a new target of `.toolchain/toolchain.toml` needs a line in each matrix.
If it finds no fault, `main` writes `no fault in <number> workflows`.

## Functions

| Function | Result |
| --- | --- |
| `faults(root, paths, targets)` | Gives the faults of the workflows `paths`, with the names of `targets`. |
| `main()` | Runs the check. |

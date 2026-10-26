# .toolchain/tools/workflows.py

`workflows.py` checks that each workflow with a matrix builds each target of the toolchain.
The command `check` runs it.

## The workflows

| Workflow | When it runs | What it does |
| --- | --- | --- |
| `ci.yml` | On each pull request, and on each push to `main`. | On each target, it runs `fetch`, the five commands and `pack`. `docs/contributing/pull_request.md`, *Before you open a pull request*, gives the five commands. |
| `cla.yml` | On each pull request, and on each new comment of a pull request. | It runs `cla`, which asks the author to sign `cla.md`, and sets the status `cla`. |
| `release.yml` | On each push of a tag `v*`. | It makes the release, as *The release* tells. |
| `toolchain.yml` | Only in the repository `my_c23_template`: on each pull request, and on each push to `main`. | On each target, it runs `fetch`, `test --toolchain` and `test --toolchain --break`. |

## The release

`release.yml` does these steps:

1. It makes a draft release, with the name of the tag.
2. On each target of its matrix, it runs `fetch`, `test` and `pack --target <target>`. Then it adds the zip of the target to the release.
3. It writes `sha256.txt`, with the sha256 of each zip, and adds it to the release.
4. It publishes the release.

A project that uses the release names the zip of each target in a `[[source]]`, with the sha256 that `sha256.txt` gives.
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

# .toolchain/tools/workflows.py

`workflows.py` checks that each workflow with a matrix builds each target of the toolchain.
The command `check` runs it.

## The workflows

| Workflow | When it runs | What it does |
| --- | --- | --- |
| `ci.yml` | On each pull request. | On each target, it runs the five commands and `pack`, as *The jobs of ci.yml* tells. `docs/contributing/pull_request.md`, *Before you open a pull request*, gives the five commands. |
| `cla.yml` | On each pull request, and on each new comment of a pull request. | It runs `cla`, which asks the author to sign `cla.md`, and sets the status `cla`. |
| `release.yml` | On each push of a tag `v*` or `*-v*`. | It makes the release of the project or of a part, as *The release* tells. |
| `warm.yml` | On each push to `dev`. | On each target, it runs `fetch` and `build`, and saves the cache of zig: *The cache of zig*. |
| `toolchain.yml` | Only in the repository `mc23t`: on each pull request. | On each target of the toolchain, it runs `fetch --toolchain`, `test --toolchain` and `test --toolchain --break`. |

No workflow runs on the push of a branch. A change reaches `dev` or `main` only through a pull request, and the pull request runs the workflows.
The step `Runner` of `ci.yml` and of `release.yml` gives the runner what the cases need: `.toolchain/docs/runner.py.md`.

## The jobs of ci.yml

The job `gate` has a job for each target and each gate: `test`, `test --break`, `test --branches` and `test --leaks`. Thus a job runs 1 gate, and stays far under its limit of 60 minutes.
Each such job prepares its runner with `runner.py`, then runs `fetch` and its gate. The job of `test` also runs `check` before its gate, and `pack` after it.

The gate runs only for the groups that `scope.py` gives for the pull request: `.toolchain/docs/scope.py.md`. A pull request into `main` runs it for the groups that state `whole = true`.
If no group tests the change, the step writes `no case group tests this change`, and runs no case.

`git diff --name-only` of the base and the head of the pull request gives the changed paths. `check` and `pack` always run for the whole project.
The step keeps the output of `scope.py` in a variable first. Thus a fault of the tool stops the step.

The variable `PYTHONUNBUFFERED` makes the runner write each line at once. Thus a job that stops at its limit still shows its last case.

The job `check` has a job for each target, with the name `check (<target>, <runner>, <architecture>)`. The rules of the branches `dev` and `main` ask for these 6 names.
Each such job waits for all the jobs of `gate`, and passes only if each of them passed.

## The cache of zig

The jobs of `ci.yml`, `release.yml` and `warm.yml` keep the global cache of zig with `actions/cache`.
Its folder is `[build] cache` of `.toolchain/toolchain.toml`.
The key is the digest of `.toolchain/toolchain.toml`, which pins the version of zig, and the target. A job that fails saves no cache.
A run of a pull request can read the caches of its base branch: https://docs.github.com/en/actions/reference/workflows-and-actions/dependency-caching.
Thus each pull request into `dev` starts with the cache of `warm.yml`.

## The release

`release.yml` runs on the tag of a release: `docs/contributing/pull_request.md`, *The release*.
It gives the tag to the tools with `--tag`. A tag `<part>-v<major>.<minor>.<patch>` names a part, and a tag `v<major>.<minor>.<patch>` names the project.
`release.yml` does these steps:

1. It makes a draft release, with the name of the tag.
2. On each target of its matrix, it prepares the runner with `runner.py`, then runs `fetch` and `pack --target <target>`. It runs no test, because the pull request from `dev` into `main` tested the tree of the tag.
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

- `<path>: the matrix does not build <target>`, for each target of the project that the matrix does not name.
- `<path>: <name> is not a target of the project`, for each name that `[project] targets` does not have.

The targets of the project are those of `[project] targets`: `.toolchain/docs/project.toml.md`, *[project]*.
The matrix of the workflow `toolchain.yml` of `.github/workflows` must name each target of `.toolchain/toolchain.toml` instead, because it tests the toolchain on each target.
Its fault for a name is then `<path>: <name> is not a target of the toolchain`.

Thus a new target of `.toolchain/toolchain.toml` needs a line in each matrix.
If it finds no fault, `main` writes `no fault in <number> workflows`.

## Functions

| Function | Result |
| --- | --- |
| `faults(root, paths, targets, every)` | Gives the faults of the workflows `paths`, with the targets of the project `targets`, and with each target of the toolchain `every` for `toolchain.yml`. |
| `main()` | Runs the check. |

# Pull requests

A pull request asks to add a change to the project.
This document tells how to write a pull request that is easy to review.

## The branches

Make a branch from `dev` for each change, and open the pull request into `dev`.
GitHub deletes the branch after the merge.
When `dev` is ready for a release, a pull request from `dev` into `main` adds it to `main`.
`docs/contributing/commits.md` tells how each pull request merges.
The rulesets of the repository protect `main` and `dev`: a change comes only through a pull request, after its required checks pass.

## The release

A release is a tag `v<major>.<minor>.<patch>` on a merge commit of `main`, for example `v0.1.1`.
The Project Owner of `cla.md` decides each release.
A release comes when the package changes: a header, a library or another file of the zip.
A change of only the toolchain, the documents or the cases makes no release.

The number follows Semantic Versioning 2.0.0: https://semver.org.
A fix that keeps the interface increments the patch.
The tag is annotated, and its message is its name:

```sh
git tag -a v0.1.1 -m v0.1.1 <commit>
git push origin v0.1.1
```

In a project with parts, each part has its own releases and its own numbers: `.toolchain/docs/project.toml.md`, *[parts]*.
The tag of a release of a part is `<part>-v<major>.<minor>.<patch>`, for example `video-v0.1.1`.
Such a project makes no tag `v<major>.<minor>.<patch>`. A release of a part comes when the package of the part changes.

```sh
git tag -a video-v0.1.1 -m video-v0.1.1 <commit>
git push origin video-v0.1.1
```

The workflow `release` then makes the release. `.toolchain/docs/workflows.py.md`, *The release*, tells its steps.

## Before you open a pull request

1. Make sure that `python project.py check` passes.
2. Make sure that the 4 test commands pass for the parts of the change. Give each one `--part <part>` for each part that the change touches, and for each part that uses such a part.
   - `python project.py test`.
   - `python project.py test --break`: each case fails, as it must.
   - `python project.py test --branches`: each branch of the code runs in a case.
   - `python project.py test --leaks`: each case gives back what the code takes.
3. If the change touches a file that belongs to no part, write in the description which parts the change reaches. Then run the 4 commands for these parts.
4. In a project with no part, run the 4 commands with no `--part`.
5. Read the full change, and remove each part that the change does not need.
6. Make sure that the documents describe the change, and that a case checks each new behavior.

The 4 commands run for the whole project when `dev` goes into `main`.
The workflow `ci` runs `check` and the 4 commands again on each host of the project: `.toolchain/docs/project.toml.md`, *[project]*.
On a pull request into `dev`, it runs the 4 commands only for the parts of the change and their users: `.toolchain/docs/scope.py.md`.
A change of a file of no part runs them for the whole project there.
Then it runs `python project.py pack`, which refuses a package that holds a path of the machine that built it.

## One purpose

A pull request has one purpose. A small pull request is easier to review and safer to merge.
If a change becomes large, divide it into smaller pull requests, each with one purpose.

## The title

The title tells the change in one line, in the form of a commit: `type(scope): description`.
`docs/contributing/commits.md` gives the types, and the rules of the description.

## The description

The description has these three headings. Write the text of each part below its heading.

```text
## Change

## Reason

## Cases
```

- Change: what the change gives to the users or to the developers.
- Reason: why the project needs the change. If the change closes an issue, write `Closes #<number>`.
- Cases: the cases that check the change.

If the change touches many files, tell the reviewers which file to read first.

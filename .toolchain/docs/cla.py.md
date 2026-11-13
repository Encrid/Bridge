# .toolchain/tools/cla.py

`cla.py` decides if the author of a pull request signed the agreement `cla.md`.
It keeps the record of the authors who signed.

```sh
python .toolchain/tools/cla.py
python .toolchain/tools/cla.py --decide <author> <signed> [<by> <said>]
```

The workflow `cla` runs `python project.py cla` when a pull request opens or changes, and when a person writes a comment on a pull request.
The workflow uses `cla.py` of the default branch. `cla.py` reads the pull request as data, and runs no code of it.

## The decision

| Author | Action | The pull request |
| --- | --- | --- |
| An account of the record, with the digest of the current `cla.md` | Nothing | passes |
| An author whose comment is only the sentence | Record | passes |
| Each other author | Ask | fails |

The sentence is `I have read cla.md and I sign it.`, with no other text in the comment.
Only a comment of the author counts. A comment of a different person does not sign for the author.
Each author of a pull request signs, with no exception.

`main` finds the author in the record by the account ID and by the digest of `cla.md`. Only then does `decide` get the author as signed.
Thus an author signs again after each change of `cla.md`.

## The record

The record is `signatures.json` on the branch `cla-signatures` of the repository. Thus the record stays with the project.
It is a list. Each item has these keys:

| Key | Value |
| --- | --- |
| `name` | The login of the author when they signed. It is for the persons who read the record. |
| `id` | The account ID of the author. GitHub never gives an ID to a second account. |
| `cla` | The SHA-256 digest of `cla.md` that the author accepted, from `agreement`. |
| `day` | The day of the signature. |
| `pull_request` | The number of the pull request of the signature. |

A login can change, and a different person can then take the old login. So the decision compares the ID, and never the login.
An item with no `id` or no `cla` comes from an earlier version of `cla.py`. It does not count, so that author signs again.
The first record makes the branch from the last commit of the default branch.

Before each write, `record` reads the record again. If a different run wrote it in between, GitHub answers 409, and `record` tries again, 3 times at most.
After 3 conflicts it stops: `signatures.json changed 3 times during the record: run the workflow cla again`.

## The question

`cla.py` asks the author one time in each pull request, in a comment:

```text
Thank you for this pull request.
Before we can merge it, @<author> must sign the agreement cla.md at the root of this repository.
The agreement is a licence. You keep the copyright of your work.
The holder can also sell the project under different terms, and these terms can include your contribution.
Read cla.md before you sign it.
To sign it, write a comment that contains only this sentence:

    I have read cla.md and I sign it.
```

`cla.py` does not ask again if a comment of the pull request already holds the question.
`asked` reads each page of the comments, 100 comments on a page, until a page with fewer comments.

## The status

`cla.py` sets the status `cla` of the last commit of the pull request: `success` if it passes, else `failure`.
The description of the status is the reason of the decision.

| Decision | Reason |
| --- | --- |
| A name of the record | `<author> has signed cla.md` |
| The sentence | `<author> signs cla.md` |
| Each other author | `<author> has not signed cla.md` |

## --decide

`--decide` writes the decision and changes nothing. The cases use it.
`<signed>` is the names of the record, with commas between them.
`<by>` is the author of a comment, and `<said>` is its text.

The output is `<action>, <passes or fails>: <reason>`, for example `nothing, passes: ann has signed cla.md`.
With a different number of values, `cla.py` stops: `usage: cla.py --decide <author> <signed> [<by> <said>]`.

## Functions

| Function | Result |
| --- | --- |
| `decide(author, signed, by, said)` | Gives the action, if the pull request passes, and the reason. |
| `call(method, path, body, empty)` | Sends one request to the API of GitHub for the repository. It gives nothing for an error of `empty`, by default 404: the thing is not there. |
| `agreement()` | Gives the SHA-256 digest of `cla.md` of the checkout, which is the default branch. |
| `signatures()` | Gives the record, and the version of its file. |
| `record(user, digest, number)` | Adds the account of the author to the record, with the digest. The first time, it makes the branch. |
| `asked(number, question)` | Tells if a comment of the pull request holds the question. |
| `main()` | Reads the event of GitHub, decides, asks or records, and sets the status. |

`call` stops if GitHub answers with an error that is not in `empty`: `<method> <path> answered <code>: <text>`.

## Limits

No case runs `main`, `call`, `agreement`, `signatures`, `record` or `asked`, because they need GitHub, a pull request or the checkout of the workflow.
The cases run `decide` through `--decide`.

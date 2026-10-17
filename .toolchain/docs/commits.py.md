# .toolchain/tools/commits.py

`commits.py` checks the messages of the commits, as `docs/contributing/commits.md` tells.

```sh
python .toolchain/tools/commits.py [--root <folder>] [--commits <range>]
```

With `--commits <range>`, `commits.py` checks each commit that `git log <range>` gives, also each merge commit.
The workflow `ci` gives the range of the push or of the pull request.

Without `--commits`, `commits.py` checks the last commit, `HEAD`, when `shared.repository` gives true and the repository has a commit.
Else it has no commit to check, and it passes. Thus a copy of the project without git passes.

## The checks

1. The message has 1 line. Thus it has no body, no trailer and no footer.
2. The line is `<type>(<scope>): <description>` or `<type>: <description>`. The scope has no space and no parenthesis.
3. The type is `feat`, `fix`, `refactor`, `chore`, `docs`, `test`, `build`, `perf`, `style`, `ci` or `revert`.
4. The description has at most 20 words. Each group of characters between 2 spaces counts.

`commits.py` cannot check that the description is a command of STE. The reviewer of the pull request checks it.

## Output

The faults come in the order that `git log` gives, and the count names commits.
The output follows `.toolchain/docs/shared.py.md`, *The report of a check*.
`<commit>` is the short name of the commit that `git` gives.

| Fault | Line |
| --- | --- |
| The message has a second line. | `<commit>: the message has more than 1 line` |
| The line has a different form. | `<commit>: the line is not type(scope): description` |
| The type is not in the list. | `<commit>: the type <type> is not in the list` |
| The description is too long. | `<commit>: the description has <count> words, and 20 is the most` |

If `git log` fails, `commits.py` stops: `git log <range>: <error>`.

## Functions

| Function | Result |
| --- | --- |
| `faults(message)` | Gives the faults of one message. |
| `main()` | Reads the options, reads the messages with `git log`, and writes the faults. |

# .toolchain/tests/cases/commits.toml

The group `commits` checks `.toolchain/tools/commits.py`.
Each case runs `.toolchain/tests/data/commits/commit.py`, which makes a git repository with some commits, then runs `commits.py` on it.

```sh
python .toolchain/tests/data/commits/commit.py <folder> <message> ... -- <command> ...
```

`commit.py` makes a git repository in `<folder>`, with one empty commit for each message, in the order of the messages.
With no message, the repository has no commit.
Then it runs the command, and ends with its exit status.

The commits do not use the configuration of git of the machine: `GIT_CONFIG_GLOBAL` names a file that is not there, and `GIT_CONFIG_NOSYSTEM` is set.
The author of the commits is `c23`, with the address `c23@example.com`.

| Case | Claim | False variant |
| --- | --- | --- |
| a message with a body | The message is `fix: correct the sum`, an empty line and `The sum was wrong.`. `commits.py` writes `the message has more than 1 line`. | The message is `fix: correct the sum`. |
| a line of a different form | The message is `Correct the sum`. `commits.py` writes `the line is not type(scope): description`. | The message is `fix: correct the sum`. |
| a type that is not in the list | The message is `feature: add the sum`. `commits.py` writes `the type feature is not in the list`. | The message is `feat: add the sum`. |
| a scope | The message is `fix(sum): correct the sum`. `commits.py` ends with 0. | The scope is `the sum`, with a space. |
| a description with too many words | The description has 21 words. `commits.py` writes `the description has 21 words, and 20 is the most`. | The description has 20 words. |
| a range of commits | The messages are `Correct the sum`, then `fix: correct the sum`. With `--commits HEAD`, `commits.py` writes `the line is not type(scope): description`. | No `--commits`. Thus `commits.py` checks the last commit only. |
| a range that git does not know | With `--commits nowhere`, `commits.py` stops: `git log nowhere:`. | `--commits HEAD`. |
| a folder with no repository | `commits.py` runs on `{case}/plain`, a folder with no repository of its own. It writes `no fault in 0 commits`. | `commits.py` runs on a repository with the commit `fix: correct the sum`. |
| a repository with no commit | `commit.py` gets no message. `commits.py` writes `no fault in 0 commits`. | `commit.py` gets the message `fix: correct the sum`. |

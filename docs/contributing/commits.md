# Commits

- The message of a commit has 1 line: `type(scope): description`.
- The type is `feat`, `fix`, `refactor`, `chore`, `docs`, `test`, `build`, `perf`, `style`, `ci` or `revert`.
- The description is a command, with at most 20 words.
- The message has no body, no trailer and no footer.

A pull request into `dev` merges by rebase. Thus each of its commits obeys these rules.
A pull request from `dev` into `main` merges with a merge commit. The line of that commit is the title of the pull request.

# Timings

This document tells how to time a commit that must keep the speed of Bridge.

## The step

Each refactor commit and each perf commit gets a timing against its parent before its review:

```sh
python tools/reference.py --pair <parent> <commit>
```

`docs/tools/reference.py.md` tells how the tool times the 2 trees, and which time is slower.

1. Run the step on an idle machine on AC power, in one power mode for the whole run.
2. Put the report under the heading `## Cases` of the description of the pull request.
3. The reviewer accepts each slower time, or refuses the commit.

`docs/contributing/pull_request.md`, *The description* gives the headings of a description.
A perf commit names the row that shows its gain. If no row times its code, its description says so.

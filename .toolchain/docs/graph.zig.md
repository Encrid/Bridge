# .toolchain/graph.zig

`graph.zig` is the build file that `zig build` runs for the runner `zig`: `.toolchain/docs/build.py.md`, *The runner zig*.
It reads the graph that `-Dgraph=<path>` names, and makes a step of zig for each step of the graph. It has no logic of its own.
It tells zig that its work depends on the content of the graph. Thus zig runs `graph.zig` again only when the graph changes.

## The graph

The graph is a JSON object with one key, `steps`: a list of steps.
Each step comes after the steps that it uses.

| Key of a step | Value |
| --- | --- |
| `name` | The name of the step. Another step names it to use its output. |
| `program` | The program that the step runs. |
| `cwd` | The folder where the program runs. |
| `args` | The arguments of the program, in their order. |
| `install` | Optional. The path in `--prefix` where zig copies the output of the step. |

Each argument has a `kind`, a `value`, and a `prefix`, the text before the value in the same argument.

| Kind | Value |
| --- | --- |
| `text` | A text, as it is. |
| `file` | The path of an input file. zig hashes its content. |
| `made` | The name of an earlier step. Its output is an input of this step, so this step waits for it. |
| `folder` | The name of an earlier step. Its output is an input of this step as a folder, with the text before the value as the prefix of the argument. |
| `output` | The name of the output file of the step, in the cache of zig. |
| `deps` | The name of the dependency file of the step. zig takes each header that it names as an input. |

zig hashes the program, the folder, the texts and the inputs of a step. `.toolchain/docs/build.py.md`, *The runner zig*, tells when a step runs again.

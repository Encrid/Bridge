# .toolchain/tools/hosts.py

`hosts.py` writes the targets of the project that a host builds, one on each line, in the order of `[targets]` of `.toolchain/toolchain.toml`.
The jobs of the workflows pack each of them: `.toolchain/docs/workflows.py.md`, *The jobs of ci.yml*.

```sh
python .toolchain/tools/hosts.py --host <host>
```

With no `--host`, the host is the target of this machine.
A target belongs to a host when its key `built_on` names the host: `.toolchain/docs/toolchain.toml.md`, *[targets.\<target\>]*.
A name that is not a host of the project stops the tool: `<name> is not a host of the project`.

## Functions

| Function | Result |
| --- | --- |
| `main()` | Reads the options, and writes the targets of the host. |

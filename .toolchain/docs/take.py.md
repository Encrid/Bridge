# .toolchain/tools/take.py

`take.py` takes the dependencies of the project.
`fetch.py` and `build.py` run it. It does not run alone.

## Functions

| Function | Result |
| --- | --- |
| `listed(path)` | Reads a list. It gives the keys of the list and the digest of each file of the list. |
| `copies(found)` | Gives each copy to write, with its bytes. It stops if a file is not as its list states. |
| `take(config, values)` | Takes each dependency, and writes what it took. |

## Lists

A line of a list is `<key> <value>`, or `<digest>  <file>`.
`listed` reads a line as a digest if its first word has 64 hexadecimal digits.
An empty line does not count.

## take

`take` does these steps:

1. If a dependency has `make`, it runs `make` first.
2. It sets the variable `{<name>}` to the `into` folder of the dependency.
3. If the list is not there and the dependency is optional, it writes `not taken: <list> is not there`.
4. If the list is not there and the dependency is not optional, it stops: `<list> is not there`.
5. It stops if the values of an `[[agree]]` are different: `refused: the dependencies do not agree`.
6. It gets the copies from `copies`, and writes them.
7. For each dependency that it took, it writes `taken: <name>`, then each key of the list and its value.

`[[agree]]` compares the first `length` characters of each value.
A dependency that `take` did not take does not count.
If a list does not have the key, its value is empty.

## Copies

`copies` does these steps for each file of each list:

1. If the copy is there and has the digest of the list, it goes to the next file.
2. If the file is not there, it stops: `<file> is not there`.
3. If the file does not have the digest, it stops: `refused: <file> is not what <list> states`.

`copies` checks all the files before `take` writes a copy.
Thus a refusal changes no file.

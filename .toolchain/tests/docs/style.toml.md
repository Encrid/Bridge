# .toolchain/tests/cases/style.toml

The group `style` checks `.toolchain/tools/style.py`.
Each case writes a small project into `{case}/p` with `files`, and runs `style.py --root {case}/p`.
`{case}/p/project.toml` gives the exceptions of the case.
In this document, each path of the small project starts with `{case}/p`.

The self-test of `style.py` checks each rule on a text that breaks the rule. Thus the cases do not check each rule again.
They check the files that `style.py` reads, and the exceptions.

| Case | Claim | False variant |
| --- | --- | --- |
| a fault of the style | `{case}/p/src/a.c` has no legal notice. `style.py` writes `src/a.c:1: G1 the file does not start with the legal notice`. | The file starts with the legal notice. |
| a notice of a different holder | The line of copyright of `{case}/p/src/a.c` names `Someone`. `style.py` ends with 0. | The notice has no line of the licence. |
| an exception | An exception excuses G1 for `{case}/p/src/a.c`, which has no legal notice. `style.py` ends with 0. | No exception. |
| an exception for one function | The body of `long_one` in `{case}/p/src/a.c` has 21 lines. An exception excuses F4 for `long_one`. `style.py` ends with 0. | The exception is for a different function. |
| an exception that excuses nothing | An exception excuses F4 for `{case}/p/src/a.c`, which has no long function. `style.py` writes `the exception for 'src/a.c' excuses nothing`. | The body of a function of the file has 21 lines. |
| an exception for a file that is not there | An exception excuses G1 for `{case}/p/src/gone.c`, which is not there. `{case}/p/src/a.c` has its legal notice. `style.py` writes `.toolchain/project.toml:1: G1 the exception for 'src/gone.c' excuses nothing`. | `{case}/p/src/gone.c` is there, with no legal notice. |
| a file of tests | `{case}/p/tests/a.c` has no legal notice. `style.py` ends with 0. | The file is `{case}/p/src/a.c`. |

## Limits

No case runs the failure of the self-test, because a case cannot change `style.py`.

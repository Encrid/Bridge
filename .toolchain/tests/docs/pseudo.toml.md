# .toolchain/tests/cases/pseudo.toml

The group `pseudo` checks `.toolchain/tools/pseudo.py`.
Each case writes the document `a.md` into `{case}/docs` with `files`, and runs `pseudo.py` on `{case}`.
The document has one block of pseudo code, which starts on its line 3.
The false variant of a case changes the block.

| Case | Claim | False variant |
| --- | --- | --- |
| the example of the profile | `folders` copies `.toolchain/docs/pseudo.py.md` into `{case}/docs`. `pseudo.py` finds no fault in it, and ends with 0. | The document `a.md` has the line `x := 1`. |
| a comment | `docs/a.md:4: the profile refuses comments`. | No `//`. |
| the arrow of the guide | `docs/a.md:4: the profile refuses the arrow of the guide: write <-`. | `<-` in place of the arrow. |
| an assignment with := | `docs/a.md:4: the profile refuses :=, write <-`. | `<-` in place of `:=`. |
| file handling | `OPENFILE`: `docs/a.md:4: the profile refuses file handling`. | A `CALL`. |
| a class | `CLASS`: `docs/a.md:4: the profile refuses classes and objects`. | A `TYPE`. |
| an input | `INPUT`: `docs/a.md:4: the profile refuses input and output`. | An assignment. |
| an output | `OUTPUT`: `docs/a.md:4: the profile refuses input and output`. | A `CALL`. |
| an indentation of two spaces | `docs/a.md:5: indented by 2, and its level needs 3`. | Three spaces. |
| a keyword in lower case | `if x > 0 THEN`: `docs/a.md:4: the keyword if is not in upper case`. | `IF`. |
| a line of no form | `x = 1`: `docs/a.md:4: not a line of the profile`. | `x <- 1`. |
| a type in lower case | `DECLARE x : integer`: `docs/a.md:4: the keyword integer is not in upper case`. | `INTEGER`. |
| a TYPE with an assignment | `docs/a.md:5: a TYPE holds only DECLARE lines`. | A `DECLARE`. |
| a clause after OTHERWISE | `docs/a.md:7: a clause after OTHERWISE`. | The clause comes before `OTHERWISE`. |
| an end that does not match | `ENDWHILE` after `IF`: `docs/a.md:6: ENDWHILE does not match the open block: IF`. | `ENDIF`. |
| NEXT with a different name | `docs/a.md:6: NEXT j does not match the open block: FOR`. | `NEXT i`. |
| an end with no block | `docs/a.md:4: ENDIF does not match the open block: none`. | An `IF` comes before the `ENDIF`. |
| a block that is not closed | `docs/a.md:4: IF is not closed`. | The block has an `ENDIF`. |

# .toolchain/tools/pseudo.py

`pseudo.py` checks each block of pseudo code in the documents of `prose.py`.
A block of pseudo code is a block of code with the tag `pseudo`.
`docs/contributing/documentation_style.md`, *Blocks of code* gives the profile of the pseudo code: its source, its changes and its layout.
This document gives the exact form of each line of the profile.

## The lines

Each line of a block has one of these forms.
A word in angle brackets is a part that the next sections describe.

```text
DECLARE <name> : <type>
CONSTANT <name> = <literal>
TYPE <name>
ENDTYPE
<target> <- <expression>
IF <expression> THEN
ELSE
ENDIF
CASE OF <target>
<expression> : <statement>
<expression> TO <expression> : <statement>
OTHERWISE : <statement>
ENDCASE
FOR <name> <- <expression> TO <expression>
FOR <name> <- <expression> TO <expression> STEP <expression>
NEXT <name>
WHILE <expression>
ENDWHILE
REPEAT
UNTIL <expression>
PROCEDURE <name>(<parameters>)
ENDPROCEDURE
FUNCTION <name>(<parameters>) RETURNS <type>
ENDFUNCTION
CALL <name>(<expressions>)
RETURN <expression>
```

`CONSTANT` takes a literal, with a `-` before it if necessary.
A parameter is `<name> : <type>`, with `BYREF` or `BYVAL` before it if necessary.
`<parameters>` and `<expressions>` are zero items or more, with a comma between two items.

A block starts with a line of this table, and ends with the line of its end:

| Start | End |
| --- | --- |
| `TYPE` | `ENDTYPE` |
| `IF` | `ENDIF`, with one `ELSE` before it if necessary |
| `CASE` | `ENDCASE` |
| `FOR` | `NEXT` and the name of the variable of the `FOR` |
| `WHILE` | `ENDWHILE` |
| `REPEAT` | `UNTIL` |
| `PROCEDURE` | `ENDPROCEDURE` |
| `FUNCTION` | `ENDFUNCTION` |

A `TYPE` holds only lines `DECLARE`.
A `CASE` holds only clauses. A clause has a value, a range or `OTHERWISE`, then `:` and one statement.
The statement of a clause is an assignment, a `CALL` or a `RETURN`.
`OTHERWISE` is the last clause of its `CASE`.

## Names, literals and types

A name starts with a letter, and holds letters, digits and `_`, as in the guide.
A keyword of the profile is not a name.
A literal is an integer, a real, a string, a character, `TRUE` or `FALSE`, for example `128`, `0.5`, `"text"` or `'a'`.
A real has digits on the two sides of its `.`, as in the guide.

A type is one of these:

- `INTEGER`, `REAL`, `CHAR`, `STRING`, `BOOLEAN` or `DATE`, as in the guide.
- `U8`, `U16`, `U32` or `U64`: an integer with no sign, of 8, 16, 32 or 64 bits.
- `I8`, `I16`, `I32` or `I64`: an integer with a sign, of 8, 16, 32 or 64 bits.
- `F32` or `F64`: a number with a floating point, of 32 or 64 bits.
- `ARRAY[<lower>:<upper>] OF <type>`. A comma separates the bounds of two dimensions, as in `ARRAY[1:3, 1:3] OF CHAR`.
- A name: a type of a `TYPE`, or a type of the code. A type of this list in lower case, such as `integer`, is not a name.

## Expressions

An expression is an operand, or two expressions with an operator between them.

| Operator | Result |
| --- | --- |
| `=`, `<>`, `<`, `<=`, `>`, `>=` | The comparison of two values. |
| `+`, `-`, `*`, `/` | The sum, the difference, the product and the quotient. |
| `DIV`, `MOD` | The quotient and the remainder of the division of two integers. |
| `AND`, `OR` | The logic operations. |
| `&` | The two strings, joined. |

An operand is one of these, with `-` or `NOT` before it if necessary:

- a literal,
- an expression in parentheses,
- a name, or a call of a function `<name>(<expressions>)`, with fields and elements after it.

A field is `.` and a name. An element of an array is `[<expressions>]`.
A call has a name before its parentheses. Thus a call of a method, such as `a.b(c)`, is not in the profile.
A target is a name with fields and elements after it, and no call.

## Refused texts

The profile refuses these texts on each line of a block:

| Text | Message |
| --- | --- |
| `//` | `the profile refuses comments` |
| The arrow of the guide, U+2190 | `the profile refuses the arrow of the guide: write <-` |
| `:=` | `the profile refuses :=, write <-` |
| `OPENFILE`, `READFILE`, `WRITEFILE`, `CLOSEFILE`, `SEEK`, `GETRECORD`, `PUTRECORD` or `EOF` | `the profile refuses file handling` |
| `CLASS`, `ENDCLASS`, `INHERITS`, `NEW`, `SUPER`, `PUBLIC` or `PRIVATE` | `the profile refuses classes and objects` |
| `INPUT` or `OUTPUT` | `the profile refuses input and output` |

`pseudo.py` does not check more of a line that has a refused text.

## Faults

Each message starts with `<path>:<line>:` and a space.

| Message | Fault |
| --- | --- |
| `indented by <number>, and its level needs <number>` | The line does not start with three spaces for each level. |
| `the keyword <word> is not in upper case` | The line has no form of the profile, and a word of the line is a keyword in lower case. |
| `not a line of the profile` | The line has no form of the profile. |
| `a TYPE holds only DECLARE lines` | A line in a `TYPE` is not `DECLARE` or `ENDTYPE`. |
| `a clause after OTHERWISE` | A clause follows the clause `OTHERWISE` of its `CASE`. |
| `<end> does not match the open block: <start>` | The end of a block is not the end of the open block. `<start>` is `none` if no block is open. |
| `<start> is not closed` | A block has no end in the block of code. |

## Example

These blocks use each form of the profile.

```pseudo
CONSTANT MOST = 6
CONSTANT NONE = -1
TYPE fault_t
   DECLARE line : INTEGER
   DECLARE what : STRING
ENDTYPE
DECLARE faults : ARRAY[1:64] OF fault_t
DECLARE grid : ARRAY[0:7, 0:7] OF U8
DECLARE count : I32
DECLARE ratio : F64
DECLARE done : BOOLEAN
```

```pseudo
IF count > MOST AND NOT done THEN
   faults[count].what <- "too long"
ELSE
   count <- count + 1
ENDIF
CASE OF faults[1].line
   0 : CALL stop()
   1 TO 9 : ratio <- (count + 1) / 10
   OTHERWISE : done <- TRUE
ENDCASE
```

```pseudo
FOR i <- 1 TO 64
   IF faults[i].line = NONE THEN
      count <- count - 1
   ENDIF
NEXT i
FOR i <- 7 TO 0 STEP -1
   grid[i, 0] <- 0
NEXT i
WHILE count > 0
   count <- count - 1
ENDWHILE
REPEAT
   count <- count + 1
UNTIL count = MOST
```

```pseudo
PROCEDURE add(BYREF total : INTEGER, BYVAL more : INTEGER)
   total <- total + more
ENDPROCEDURE
FUNCTION greeting(name : STRING) RETURNS STRING
   RETURN "hello, " & name
ENDFUNCTION

PROCEDURE greet()
   CALL add(count, LENGTH(greeting(user())))
ENDPROCEDURE
```

## Functions

Each function that reads tokens gets the tokens `t` of a line and an index `i`.
It gives the index after the last token that it read.
It raises `ValueError` through `need` if the tokens do not have its form.

| Function | Result |
| --- | --- |
| `at(t, i)` | Gives the token at `i`, or an empty text after the last token. |
| `need(holds, i)` | Gives `i` if `holds` is true. Else it raises `ValueError`. |
| `want(t, i, token)` | Reads the token `token`. |
| `named(t, i)` | Reads a name. |
| `listed(t, i, item, end)` | Reads items with a comma between two items, then the token `end`. `item` reads one item. |
| `operand(t, i)` | Reads an operand. |
| `selected(t, i)` | Reads the fields and the elements after a name or a call. |
| `expression(t, i)` | Reads an expression. |
| `typed(t, i)` | Reads a type. |
| `bounds(t, i)` | Reads the bounds of one dimension of an array. |
| `parameter(t, i)` | Reads a parameter. |
| `statement(t)` | Reads a line that is not a clause. |
| `clause(t)` | Reads a clause of a `CASE`. |
| `checked(lines, first)` | Gives the faults of the lines of a block. `first` is the number of the first line in the document. |
| `main()` | Runs the check. |

## Output

The output follows `.toolchain/docs/shared.py.md`, *The report of a check*.

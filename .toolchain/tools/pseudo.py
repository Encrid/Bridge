# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re

from prose import blocks, documents
from shared import arguments, report

TYPES = {'INTEGER', 'REAL', 'CHAR', 'STRING', 'BOOLEAN', 'DATE', 'U8', 'U16', 'U32', 'U64', 'I8', 'I16', 'I32',
         'I64', 'F32', 'F64'}
BINARY = {'=', '<>', '<', '<=', '>', '>=', '+', '-', '*', '/', '&', 'DIV', 'MOD', 'AND', 'OR'}
BLOCKS = {'TYPE': {'ENDTYPE'}, 'IF': {'ELSE', 'ENDIF'}, 'ELSE': {'ENDIF'}, 'CASE': {'ENDCASE'}, 'FOR': set(),
          'WHILE': {'ENDWHILE'}, 'REPEAT': {'UNTIL'}, 'PROCEDURE': {'ENDPROCEDURE'}, 'FUNCTION': {'ENDFUNCTION'}}
CLOSERS = {'ENDTYPE', 'ELSE', 'ENDIF', 'ENDCASE', 'NEXT', 'ENDWHILE', 'UNTIL', 'ENDPROCEDURE', 'ENDFUNCTION'}
ALONE = {'ELSE', 'ENDIF', 'ENDTYPE', 'ENDCASE', 'ENDWHILE', 'REPEAT', 'ENDPROCEDURE', 'ENDFUNCTION'}
KEYWORDS = TYPES | BINARY | set(BLOCKS) | CLOSERS | {'DECLARE', 'CONSTANT', 'THEN', 'OF', 'OTHERWISE', 'TO', 'STEP',
                                                     'RETURNS', 'CALL', 'RETURN', 'BYREF', 'BYVAL', 'NOT', 'TRUE',
                                                     'FALSE', 'ARRAY'}
REFUSED = dict({'//': 'comments', '\u2190': 'the arrow of the guide: write <-', ':=': ':=, write <-',
                'INPUT': 'input and output', 'OUTPUT': 'input and output'},
               **dict.fromkeys(('OPENFILE', 'READFILE', 'WRITEFILE', 'CLOSEFILE', 'SEEK', 'GETRECORD', 'PUTRECORD',
                                'EOF'), 'file handling'),
               **dict.fromkeys(('CLASS', 'ENDCLASS', 'INHERITS', 'NEW', 'SUPER', 'PUBLIC', 'PRIVATE'),
                               'classes and objects'))
TOKEN = re.compile(r'\d+(?:\.\d+)?|"[^"]*"|\'[^\']\'|[A-Za-z]\w*|<-|<>|<=|>=|:=|//|\S', re.A)
NAME = re.compile(r'[A-Za-z]\w*', re.A)
LITERAL = re.compile(r'\d+(?:\.\d+)?|"[^"]*"|\'[^\']\'|TRUE|FALSE')


def at(t, i):
    return t[i] if i < len(t) else ''


def need(holds, i):
    if not holds:
        raise ValueError
    return i


def want(t, i, token):
    return need(at(t, i) == token, i + 1)


def named(t, i):
    return need(NAME.fullmatch(at(t, i)) and at(t, i) not in KEYWORDS, i + 1)


def listed(t, i, item, end):
    i = item(t, i)
    while at(t, i) == ',':
        i = item(t, i + 1)
    return want(t, i, end)


def operand(t, i):
    while at(t, i) in ('-', 'NOT'):
        i += 1
    if LITERAL.fullmatch(at(t, i)):
        return i + 1
    if at(t, i) == '(':
        return want(t, expression(t, i + 1), ')')
    i = named(t, i)
    if at(t, i) == '(':
        i = i + 2 if at(t, i + 1) == ')' else listed(t, i + 1, expression, ')')
    return selected(t, i)


def selected(t, i):
    while at(t, i) in ('.', '['):
        i = named(t, i + 1) if at(t, i) == '.' else listed(t, i + 1, expression, ']')
    return i


def expression(t, i):
    i = operand(t, i)
    while at(t, i) in BINARY:
        i = operand(t, i + 1)
    return i


def typed(t, i):
    if at(t, i) in TYPES:
        return i + 1
    if at(t, i) == 'ARRAY':
        return typed(t, want(t, listed(t, want(t, i + 1, '['), bounds, ']'), 'OF'))
    return named(t, need(at(t, i).upper() not in TYPES, i))


def bounds(t, i):
    return expression(t, want(t, expression(t, i), ':'))


def parameter(t, i):
    if at(t, i) in ('BYREF', 'BYVAL'):
        i += 1
    return typed(t, want(t, named(t, i), ':'))


def statement(t):
    head = at(t, 0)
    if head == 'DECLARE':
        return typed(t, want(t, named(t, 1), ':'))
    if head == 'CONSTANT':
        i = want(t, named(t, 1), '=')
        if at(t, i) == '-':
            i += 1
        return need(LITERAL.fullmatch(at(t, i)), i + 1)
    if head in ('TYPE', 'NEXT'):
        return named(t, 1)
    if head == 'IF':
        return want(t, expression(t, 1), 'THEN')
    if head in ('WHILE', 'UNTIL', 'RETURN'):
        return expression(t, 1)
    if head == 'CASE':
        return selected(t, named(t, want(t, 1, 'OF')))
    if head == 'FOR':
        i = expression(t, want(t, expression(t, want(t, named(t, 1), '<-')), 'TO'))
        return expression(t, i + 1) if at(t, i) == 'STEP' else i
    if head in ('PROCEDURE', 'FUNCTION', 'CALL'):
        i = want(t, named(t, 1), '(')
        i = i + 1 if at(t, i) == ')' else listed(t, i, expression if head == 'CALL' else parameter, ')')
        return typed(t, want(t, i, 'RETURNS')) if head == 'FUNCTION' else i
    if head in ALONE:
        return 1
    return expression(t, want(t, selected(t, named(t, 0)), '<-'))


def clause(t):
    i = 1 if at(t, 0) == 'OTHERWISE' else expression(t, 0)
    if at(t, i) == 'TO':
        i = expression(t, i + 1)
    i = want(t, i, ':')
    return need(at(t, i) not in KEYWORDS - {'CALL', 'RETURN'}, i) + statement(t[i:])


def checked(lines, first):
    found, stack = [], [['none', set(), 0]]
    for n, line in enumerate(lines, first):
        t = TOKEN.findall(line)
        refused = [token for token in t if token in REFUSED]
        if refused:
            found.append((n, 'the profile refuses ' + REFUSED[refused[0]]))
            continue
        if not t:
            continue
        head, top = t[0], stack[-1]
        level = max(len(stack) - 1 - (head in CLOSERS), 0)
        indent = len(line) - len(line.lstrip(' '))
        if indent != 3 * level:
            found.append((n, 'indented by %d, and its level needs %d' % (indent, 3 * level)))
        try:
            if top[0] == 'TYPE' and head not in ('DECLARE', 'ENDTYPE'):
                found.append((n, 'a TYPE holds only DECLARE lines'))
            elif top[0] in ('CASE', 'OTHERWISE') and head != 'ENDCASE':
                if top[0] == 'OTHERWISE':
                    found.append((n, 'a clause after OTHERWISE'))
                need(clause(t) == len(t), 0)
                if head == 'OTHERWISE':
                    top[0] = 'OTHERWISE'
            else:
                need(statement(t) == len(t), 0)
        except ValueError:
            lower = [word for word in t if word != word.upper() and word.upper() in KEYWORDS]
            found.append((n, 'the keyword %s is not in upper case' % lower[0] if lower else 'not a line of the profile'))
        if head in CLOSERS:
            key = ' '.join(t[:2]) if head == 'NEXT' else head
            if key in top[1]:
                stack.pop()
            else:
                found.append((n, '%s does not match the open block: %s' % (key, top[0])))
        if head in BLOCKS:
            stack.append([head, {'NEXT ' + at(t, 1)} if head == 'FOR' else BLOCKS[head], n])
    return found + [(n, '%s is not closed' % word) for word, _, n in stack[1:]]


def main():
    root = arguments().root
    read = documents(root)
    found = [(path, n, what) for path, lines in read for start, end, tag in blocks(lines) if tag == 'pseudo'
             for n, what in checked(lines[start + 1:end], start + 2)]
    report(['%s:%d: %s' % fault for fault in sorted(found)], len(read))


if __name__ == '__main__':
    main()

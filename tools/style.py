"""The coding style, held: docs/style.md, rule by rule, for every rule a program can hold.

    python tools/style.py              the paths that are held clean, and the self-test
    python tools/style.py --all        every C file of ours, with a count for each rule
    python tools/style.py PATH ...     these files

Reads C as text: comments and literals are set aside, functions are found by their braces, and
a function's body is cut into statements. It is no compiler, and it does not need one.

Not held, since no program reading text can: F1 and O3's coherence, the verb F2 asks a name
for, V2, A1, A2, and whether a ternary is simple (C2). A reader holds those.

An infringement that docs/style.md, *Exceptions*, lists is not reported, and an exception that
excuses nothing is. The paths in HELD only ever grow: a file that was made clean stays clean.
"""
import fnmatch
import re
import subprocess
import sys
from pathlib import Path

from c23 import OURS

ROOT = Path(__file__).resolve().parent.parent
# what has been made to follow the style, and is held to it by the tests
HELD = ('include/bridge_schema.h', 'include/bridge_abi.h', 'include/bridge_backend.h', 'include/bridge.h', 'utils/*', 'src/backend.*', 'src/pool*', 'src/schema*', 'src/config*', 'src/say.*', 'src/bridge_*', 'src/schedule*', 'src/region*', 'src/block*', 'src/inside.h', 'src/storage*', 'src/abi_*', 'src/abi.h')

COLUMNS, LINES, FUNCTIONS, ARGUMENTS, BRANCHES, DEPTH = 80, 20, 5, 4, 3, 2
SNAKE = re.compile(r'[a-z_][a-z0-9_]*\Z')
UPPER = re.compile(r'[A-Z_][A-Z0-9_]*\Z')
CONTROL = ('if', 'while', 'for', 'switch')
KEYWORDS = CONTROL + ('else', 'do', 'return', 'sizeof', 'case', 'default', 'goto', 'break',
                      'continue', 'alignof', 'static_assert', 'typeof')
BASE = (r'void|char|short|int|long|float|double|bool|auto|size_t|ptrdiff_t|signed|unsigned|'
        r'\w+_t|struct\s+\w+|enum\s+\w+|union\s+\w+')
# inside a function, where two names in a row can only be a type and what it declares
ANY = BASE + r'|[A-Za-z_]\w*'
QUALIFIER = r'static|const|constexpr|volatile|register|thread_local|extern|inline'
# a statement that declares a variable: qualifiers, a type, stars, a name
DECLARES = re.compile(r'(?:\[\[[^\]]*\]\]\s*)*(?:(?:%s)\s+)*(?:%s)\b(?:\s+(?:%s|const|long|int|char))*'
                      r'(?:[\s*]|const\b)*(?:\(\s*\*\s*)?(?P<name>[A-Za-z_]\w*)\s*'
                      r'(?:\)\s*\([^)]*\)\s*)?(?:\[[^\]]*\]\s*)*(?:=|,|\Z)'
                      % (QUALIFIER, ANY, BASE))
HEADER = re.compile(r'/\*\n\*\* Copyright \(C\) \d{4} Heuretes-Workshop\n(?:\*\* Copyright \(C\) [^\n]+\n)*'
                    r'\*\*\n\*\* This file is part of Bridge-ASM700\.\n\*\*\n\*\* All rights reserved\.\n'
                    r'\*\*\n\*\* (?P<path>\S+) -- [^\n]+\n(?:\*\*(?: [^\n]+)?\n)*\*/\n')
ASSIGNS = re.compile(r'(?<![=!<>+\-*/%&|^])=(?!=)')


class Source:
    """One file: its lines, and the same text with comments and literals set aside."""

    def __init__(self, path, text):
        self.path, self.text = path, text
        self.lines = text.split('\n')
        self.comments = []                      # (first line, last line, text)
        self.code = self.silenced()
        self.found = []

    def silenced(self):
        """Comments become spaces and a literal keeps its quotes around a filler, so that
        nothing in either is read as code; and a line of the pre-processor is set aside whole."""
        out = []
        pattern = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\\n])*"|(?<![0-9A-Za-z])\'(?:\\.|[^\'\\\n])*\'', re.S)
        last = 0
        for m in pattern.finditer(self.text):
            out.append(self.text[last:m.start()])
            body = m.group(0)
            if body[0] == '/':
                first = self.text.count('\n', 0, m.start()) + 1
                self.comments.append((first, first + body.count('\n'), body))
                out.append(re.sub(r'[^\n]', ' ', body))
            else:
                out.append(body[0] + 's' * (len(body) - 2) + body[0])
            last = m.end()
        out.append(self.text[last:])
        lines = ''.join(out).split('\n')
        self.directives = {}                    # line number: the directive as written
        carried = False
        for n, line in enumerate(lines):
            if carried or line.lstrip().startswith('#'):
                self.directives[n + 1] = self.lines[n]
                carried = line.rstrip().endswith('\\')
                lines[n] = ' ' * len(line)
        code = '\n'.join(lines)
        # extern "C" { and the brace that closes it are a C++ reader's, and no scope of ours
        code = re.sub(r'extern\s+"s*"\s*\{', lambda m: ' ' * len(m.group(0)), code)
        for n, line in self.directives.items():
            if '__cplusplus' in line and n + 1 <= len(lines) and lines[n].strip() == '}':
                at = sum(len(x) + 1 for x in lines[:n])
                code = code[:at] + ' ' * len(lines[n]) + code[at + len(lines[n]):]
        return code

    def line_of(self, at):
        return self.code.count('\n', 0, at) + 1

    def say(self, rule, line, what, function=None):
        self.found.append((self.path, line, rule, what, function))


def tokens_of(code, start, end):
    return [(m.group(0), m.start()) for m in
            re.compile(r'[A-Za-z_]\w*|\d[\w.]*|"s*"|\'s*\'|\[\[|\]\]|\S').finditer(code, start, end)]


def closing(code, at):
    """Where the bracket that opens at `at` closes."""
    pairs = {'(': ')', '[': ']', '{': '}'}
    depth = 0
    for i in range(at, len(code)):
        if code[i] in pairs:
            depth += 1
        elif code[i] in pairs.values():
            depth -= 1
            if not depth:
                return i
    return len(code) - 1


def opening(code, at):
    """Where the bracket that closes at `at` opened."""
    depth = 0
    for i in range(at, -1, -1):
        if code[i] in ')]}':
            depth += 1
        elif code[i] in '([{':
            depth -= 1
            if not depth:
                return i
    return 0


def top_level(text, what):
    """Whether `what`, a single character, stands in `text` outside every bracket."""
    depth = 0
    for c in text:
        if c in '([{':
            depth += 1
        elif c in ')]}':
            depth -= 1
        elif c == what and not depth:
            return True
    return False


def constant_in(text):
    """Whether what a declaration declares is constant: for a pointer, the pointer itself."""
    head = text.split('=')[0]
    return re.search(r'\b(?:const|constexpr)\b', head.rsplit('*', 1)[-1] if '*' in head else head)


def parameters(text):
    """How many parameters a list between parentheses states: -1 for an empty one."""
    text = text.strip()
    if not text:
        return -1
    if text == 'void':
        return 0
    depth, count = 0, 1
    for c in text:
        if c in '([{':
            depth += 1
        elif c in ')]}':
            depth -= 1
        elif c == ',' and not depth:
            count += 1
    return count


class Function:
    def __init__(self, name, start, params, open_at, close_at, prefix, macro):
        self.name, self.start, self.params = name, start, params
        self.open_at, self.close_at, self.prefix, self.macro = open_at, close_at, prefix, macro


def file_scope(src):
    """The file cut at its outermost level: functions, and every other statement."""
    code = src.code
    functions, statements = [], []
    at = begun = 0
    while at < len(code):
        c = code[at]
        if c == ';':
            statements.append((begun, at))
            begun = at + 1
        elif c == '{':
            end = closing(code, at)
            before = code[begun:at].rstrip()
            if before.endswith(')') and not ASSIGNS.search(before):
                paren = opening(code, begun + len(before) - 1)
                name = re.search(r'([A-Za-z_]\w*)\s*\Z', code[begun:paren])
                if name and name.group(1) not in KEYWORDS:
                    word = name.group(1)
                    macro = UPPER.match(word) is not None
                    params = code[paren + 1:begun + len(before) - 1]
                    functions.append(Function(params.strip() if macro else word,
                                              begun + len(code[begun:at]) - len(code[begun:at].lstrip()),
                                              params, at, end, code[begun:paren], macro))
                    at = end + 1
                    begun = at
                    continue
            at = end + 1
            continue
        at += 1
    return functions, statements


def blank_lines(src, first, last):
    return [n for n in range(first, last + 1) if not src.lines[n - 1].strip()]


def check_lines(src):
    """What a line shows of itself: F3, L2's tabs, L3."""
    code_lines = src.code.split('\n')
    for n, line in enumerate(src.lines, 1):
        if len(line.expandtabs(8)) > COLUMNS:
            src.say('F3', n, 'the line is %d columns, and %d is the most' % (len(line.expandtabs(8)), COLUMNS))
        if '\t' in line[:len(line) - len(line.lstrip())]:
            src.say('L2', n, 'a tabulation is used for indentation')
        if line != line.rstrip():
            src.say('L3', n, 'the line ends in white space')
        code = code_lines[n - 1]
        if not code.strip():
            continue
        if re.search(r',(?=[^\s])', code):
            src.say('L3', n, 'no space after a comma')
        if re.search(r'\b(?:if|while|for|switch)\(', code) or re.search(r'\breturn[^\s;]', code):
            src.say('L3', n, 'no space after a keyword')
        if re.search(r'\bsizeof\s', code):
            src.say('L3', n, 'sizeof is a unary operator: no space after it, sizeof(x)')
        call = re.search(r'\b([a-z_]\w*)\s+\((?!\s*\*)', code)
        if call and call.group(1) not in KEYWORDS and not re.match(r'(?:%s|%s)\Z' % (BASE, QUALIFIER), call.group(1)):
            src.say('L3', n, "a space between the name '%s' and its parenthesis" % call.group(1))
        if re.search(r'[!~]\s', code) or re.search(r'(?:\+\+|--)\s+[\w(*]', code) or re.search(r'[\w)\]]\s+(?:\+\+|--)', code):
            src.say('L3', n, 'a space between a unary operator and what it is applied to')
        spaced = code.replace('->', '  ')
        for m in re.finditer(r'<<=|>>=|[=!<>+\-*/%&|^]=|&&|\|\||<<|>>|[<>?/%^]|(?<![=!<>+\-*/%&|^])=(?!=)', spaced):
            before, after = spaced[:m.start()], spaced[m.end():]
            if (before.strip() and not before.endswith(' ')) or (after.strip() and not after.startswith(' ')):
                src.say('L3', n, "no space on both sides of '%s'" % m.group(0))
                break
        if re.search(r'[\w\]][+\-*&|][\w(]|\)[+|][\w(]', re.sub(r'\d[eE][+-]\d', 'x', spaced)) and not re.search(r'\+\+|--', spaced):
            if not re.search(r'\(\s*\w[\w\s]*\*+\s*\)', spaced):
                src.say('L3', n, 'a binary operator with no space on both sides')
        if re.search(r'[A-Za-z0-9_]\*+\s+[A-Za-z_(]', code) and re.search(r'\b(?:%s)\*' % BASE, code):
            src.say('V3', n, 'the pointer symbol is attached to the type, and belongs on the name')
        if re.search(r'\b(?:%s)\s+\*+\s+[A-Za-z_]' % BASE, code):
            src.say('V3', n, 'the pointer symbol stands apart from the name it belongs to')


def check_file(src, functions):
    """What a file is as a whole: O3, O4, G1, G2, G3, H2, H3, and macros' names."""
    path = Path(src.path)
    for part in path.parts[:-1] + (path.stem,):
        if not SNAKE.match(part):
            src.say('O4', 1, "'%s' is not in snake_case" % part)
    header = HEADER.match(src.text)
    if not header:
        src.say('G1', 1, 'the file does not start with the standard header')
    elif header.group('path') != src.path:
        src.say('G1', 1, "the header says '%s', and the file is '%s'" % (header.group('path'), src.path))
    if len(functions) > FUNCTIONS:
        src.say('O3', src.line_of(functions[FUNCTIONS].start),
                'the file has %d functions, and %d is the most' % (len(functions), FUNCTIONS))
    for f in functions:
        after = src.line_of(f.close_at) + 1
        gap = 0
        while after + gap <= len(src.lines) and not src.lines[after + gap - 1].strip():
            gap += 1
        if after + gap <= len(src.lines) and gap != 1:
            src.say('G2', after, '%d empty lines after a function, and it is one' % gap, f.name)
    depth = 0
    for n in sorted(src.directives):
        line = src.directives[n]
        word = re.match(r'\s*#\s*(\w+)', line)
        if not word or src.directives.get(n - 1, '').rstrip().endswith('\\'):
            continue
        if word.group(1) in ('endif', 'else', 'elif', 'elifdef', 'elifndef'):
            depth -= 1
        if len(line) - len(line.lstrip()) != 4 * max(depth, 0) or line.lstrip()[1:2] == ' ':
            src.say('G3', n, 'the directive is not indented to its level, which is %d' % max(depth, 0))
        if word.group(1) in ('if', 'ifdef', 'ifndef', 'else', 'elif', 'elifdef', 'elifndef'):
            depth += 1
        define = re.match(r'\s*#\s*define\s+(\w+)(\(?)\s*(.*)', line)
        if define:
            name, takes, body = define.groups()
            more = n
            while src.directives[more].rstrip().endswith('\\') and more + 1 in src.directives:
                more += 1
                body += ' ' + src.directives[more]
            if not UPPER.match(name):
                src.say('V1', n, "the macro '%s' is not in UPPER_CASE" % name)
            if not takes and re.match(r'[-+(]*(?:\d|"|\')', body.strip()):
                src.say('H3', n, "the macro '%s' is a constant" % name)
            whole = ' '.join(body.replace('\\', ' ').split())
            fields = not takes and whole.startswith('{') and whole.endswith('}')
            if ';' in re.sub(r'"[^"]*"', '', body) and not fields:
                src.say('H3', n, "the macro '%s' is more than one statement" % name)
            if path.suffix == '.c':
                src.say('H1', n, "the macro '%s' is defined in a source file" % name)
    if path.suffix == '.h':
        opened = [src.directives[n] for n in sorted(src.directives)][:2]
        guarded = len(opened) == 2 and re.match(r'\s*#\s*ifndef\s+(\w+)', opened[0]) \
            and re.match(r'\s*#\s*define\s+(\w+)\s*$', opened[1])
        if not guarded and not any('pragma once' in d for d in opened):
            src.say('H2', 1, 'the header is not protected from double inclusion')


def check_outside(src, statements, functions, declared):
    """What stands outside every function: G4, G5, F5, V1, H1, L4 of a type's braces."""
    code = src.code
    in_header = src.path.endswith('.h')
    for begun, end in statements:
        text = ' '.join(code[begun:end].split())
        line = src.line_of(begun + len(code[begun:end]) - len(code[begun:end].lstrip()))
        if not text:
            continue
        bare = re.sub(r'\[\[[^\]]*\]\]\s*', '', text)
        if bare.startswith('typedef'):
            last = re.search(r'(\w+)\s*(?:\[[^\]]*\])?\Z', bare)
            name = last if '{' in bare else re.search(r'\(\s*\*\s*(\w+)\s*\)\s*\(', bare) or last
            if name and not name.group(1).endswith('_t'):
                src.say('V1', line, "the type '%s' does not end with _t" % name.group(1))
            if not in_header:
                src.say('H1', line, 'a type is declared in a source file')
        elif re.match(r'(?:struct|enum|union)\b[^=(]*\{', bare):
            if not in_header:
                src.say('H1', line, 'a type is declared in a source file')
        elif re.match(r'static_assert\b', bare):
            pass
        elif re.match(r'[A-Z_][A-Z0-9_]*\s*\(', bare):
            pass
        elif re.search(r'\)\s*\Z', bare) and not ASSIGNS.search(bare):
            if not in_header:
                src.say('H1', line, 'a function is declared in a source file')
        elif DECLARES.match(bare):
            name = DECLARES.match(bare).group('name')
            constant = constant_in(bare)
            if not constant:
                src.say('G4', line, "'%s' is a global variable that is not constant" % name)
            elif not UPPER.match(name):
                src.say('V1', line, "the global constant '%s' is not in UPPER_CASE" % name)
        for m in re.finditer(r'\benum\b[^{;]*\{([^}]*)\}', bare):
            for item in m.group(1).split(','):
                name = item.split('=')[0].strip()
                if name and not UPPER.match(name):
                    src.say('V1', line, "'%s', of an enum, is not in UPPER_CASE" % name)
    for m in re.finditer(r'\(\s*\*\s*\w+\s*\)\s*\(([^()]*)\)', code):
        check_parameters(src, src.line_of(m.start()), m.group(1), None)
    for begun, end in statements:
        text = code[begun:end]
        m = re.search(r'\b([A-Za-z_]\w*)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*\Z', text)
        if m and not ASSIGNS.search(text) and not re.match(r'\s*(?:typedef|static_assert)\b', text) \
                and m.group(1) not in KEYWORDS and not UPPER.match(m.group(1)):
            check_parameters(src, src.line_of(begun + m.start()), m.group(2), m.group(1))
    for f in functions:
        static = re.search(r'\bstatic\b', f.prefix)
        exported = re.search(r'\b\w*(?:EXPORT|_API)\b|\bdllexport\b|\bvisibility\b', f.prefix) or f.macro
        if in_header and not re.search(r'\bstatic\s+inline\b|\binline\s+static\b', f.prefix):
            src.say('H1', src.line_of(f.start), "'%s' is defined in a header and is not static inline" % f.name, f.name)
        if not in_header and re.search(r'\binline\b', f.prefix):
            src.say('H1', src.line_of(f.start), "'%s' is static inline in a source file" % f.name, f.name)
        if not in_header and not static and not exported and f.name != 'main' and f.name not in declared:
            src.say('G5', src.line_of(f.start), "'%s' is used in this file alone, and is not static" % f.name, f.name)


def check_parameters(src, line, params, name):
    count = parameters(params)
    if count < 0:
        src.say('F5', line, 'a function that takes nothing takes void', name)
    elif count > ARGUMENTS:
        src.say('F5', line, '%d arguments, and %d is the most' % (count, ARGUMENTS), name)


def check_function(src, f):
    """A function from its name to its last brace: F2 to F6, L1, L2, L4 to L6, C1 to C3."""
    code = src.code
    first, opens, closes = src.line_of(f.start), src.line_of(f.open_at), src.line_of(f.close_at)

    def say(rule, line, what):
        src.say(rule, line, what, f.name)

    if not SNAKE.match(f.name):
        say('F2', first, "the function '%s' is not in snake_case" % f.name)
    if not f.macro:
        check_parameters(src, first, f.params, f.name)
    if closes - opens - 1 > LINES:
        say('F4', opens, 'the body is %d lines, and %d is the most' % (closes - opens - 1, LINES))
    if src.lines[opens - 1].strip() != '{':
        say('L4', opens, "a function's opening bracket is alone on its line")
    if src.lines[closes - 1].strip() != '}':
        say('L4', closes, "a function's closing bracket is alone on its line")
    for start, end, text in src.comments:
        if opens <= start and end <= closes:
            say('F6', start, 'a comment inside a function')
        elif end == first - 1 and not re.match(r'/\*\n\*\* %s\n\*\*\n(?:\*\*(?: [^\n]+)?\n)+\*/\Z' % re.escape(f.name), text):
            say('F6', start, "the comment above '%s' is not: its name, an empty line, what it does" % f.name)

    if not f.macro and not re.search(r'\bstatic\b', f.prefix) and not any(end == first - 1 for _, end, _ in src.comments):
        say('F6', first, "'%s' is not static, and has no comment above it" % f.name)

    tokens = tokens_of(code, f.open_at + 1, f.close_at)
    blocks = [{'level': 1, 'case': 0, 'opened': 0, 'control': 0, 'do': False}]
    state = {'extra': 0, 'chain': {}, 'declared': [], 'coded': False, 'ended': 0}

    def level():
        return blocks[-1]['level'] + blocks[-1]['case'] + state['extra']

    def starts(at, expected):
        """A statement that begins its line is indented to its level."""
        n = src.line_of(at)
        line = src.code.split('\n')[n - 1] if False else src.lines[n - 1]
        column = at - (code.rfind('\n', 0, at) + 1)
        if not line[:column].strip():
            if column != 4 * expected:
                say('L2', n, 'indented by %d, and its level is %d' % (column, expected))
        elif n == state['ended']:
            say('L1', n, 'more than one statement on the line')
        return n

    def nested():
        return sum(b['control'] for b in blocks) + state['extra']

    i = 0
    while i < len(tokens):
        word, at = tokens[i]
        if word == '}':
            block = blocks.pop() if len(blocks) > 1 else blocks[0]
            n = src.line_of(at)
            if src.lines[n - 1][:at - (code.rfind('\n', 0, at) + 1)].strip():
                say('L4', n, 'a closing bracket is not first on its line')
            elif at - (code.rfind('\n', 0, at) + 1) != 4 * block['opened']:
                say('L2', n, 'a closing bracket is not at the level of what it closes')
            state['extra'] = 0
            i += 1
            if block['do'] and i < len(tokens) and tokens[i][0] == 'while':
                end = closing(code, tokens[i + 1][1])
                while i < len(tokens) and tokens[i][1] <= end + 1:
                    i += 1
            elif i < len(tokens) and src.line_of(tokens[i][1]) == n and tokens[i][0] != 'else':
                say('L4', n, 'a closing bracket is followed by something that is not an else')
            state['ended'] = 0
            continue
        if word == '{':
            starts(at, level())
            blocks.append({'level': level() + 1, 'case': 0, 'opened': level(), 'control': 0, 'do': False})
            state['extra'] = 0
            i += 1
            continue
        if i + 2 < len(tokens) and tokens[i + 1][0] == ':' and tokens[i + 2][0] != ':' \
                and re.match(r'[A-Za-z_]\w*\Z', word) and word not in ('case', 'default') \
                and state['ended'] != src.line_of(at):
            i += 2
            continue
        if word in ('case', 'default'):
            blocks[-1]['case'] = 0
            starts(at, level())
            blocks[-1]['case'] = 1
            while i < len(tokens) and tokens[i][0] != ':':
                i += 1
            i += 1
            continue
        if word in CONTROL or word in ('else', 'do'):
            mine = level()
            n = starts(at, mine)
            last = at + len(word) - 1
            kind = word
            if word == 'else' and i + 1 < len(tokens) and tokens[i + 1][0] == 'if':
                i += 1
                kind = 'else if'
            if kind != 'else' and kind != 'do':
                last = closing(code, tokens[i + 1][1])
                head = code[tokens[i + 1][1]:last + 1]
                if kind != 'for' and ASSIGNS.search(head):
                    say('L1', n, 'a condition and an assignment on one line')
                if head.count('?') > 1:
                    say('C2', n, 'nested or chained ternaries')
            if word == 'if':
                state['chain'][mine] = 1
            elif word == 'else':
                state['chain'][mine] = state['chain'].get(mine, 1) + 1
                if state['chain'][mine] == BRANCHES + 1:
                    say('C1', n, 'more than %d branchings in one if' % BRANCHES)
            if word != 'else' and nested() + 1 > DEPTH:
                say('C1', n, 'conditional branchings nested %d deep, and %d is the most' % (nested() + 1, DEPTH))
            while i < len(tokens) and tokens[i][1] <= last:
                i += 1
            if i < len(tokens) and tokens[i][0] == '{':
                brace = tokens[i][1]
                if src.line_of(brace) != src.line_of(last):
                    say('L4', src.line_of(brace), 'an opening bracket is at the end of the line it belongs to')
                elif src.lines[src.line_of(brace) - 1].rstrip()[-1:] != '{' and code[brace + 1:code.find('\n', brace)].strip():
                    say('L4', src.line_of(brace), 'an opening bracket is not at the end of its line')
                blocks.append({'level': mine + 1, 'case': 0, 'opened': mine, 'control': 1, 'do': kind == 'do'})
                state['extra'] = 0
                i += 1
            else:
                if i < len(tokens) and src.line_of(tokens[i][1]) == src.line_of(last) and tokens[i][0] != ';':
                    say('L1', n, 'a condition and its statement on one line')
                state['extra'] += 1
            state['coded'] = True
            state['ended'] = 0
            continue
        # a statement: to its semicolon, with whatever brackets it holds
        mine = level()
        n = starts(at, mine)
        end = at
        depth = 0
        while end < f.close_at:
            c = code[end]
            if c in '([{':
                depth += 1
            elif c in ')]}':
                depth -= 1
            elif c == ';' and not depth:
                break
            end += 1
        text = ' '.join(code[at:end].split())
        if text.startswith('goto') and re.match(r'goto\b', text):
            say('C3', n, 'goto')
        declares = DECLARES.match(text) and not re.match(r'(?:return|goto|break|continue)\b', text)
        if declares:
            if len(blocks) > 1 or state['extra'] or state['coded']:
                say('L5', n, 'a variable is declared after the beginning of the function')
            else:
                state['declared'].append(src.line_of(end))
            if top_level(text, ','):
                say('L5', n, 'more than one variable declared on a line')
            name = DECLARES.match(text).group('name')
            if not SNAKE.match(name) and not (UPPER.match(name) and constant_in(text)):
                say('V1', n, "'%s' is not in snake_case" % name)
        elif text:
            state['coded'] = True
            if top_level(text, ','):
                say('L1', n, 'several statements joined by a comma')
            if len(ASSIGNS.findall(re.sub(r'\([^()]*\)|\[[^\]]*\]|\{[^{}]*\}', '', text))) > 1:
                say('L1', n, 'several assignments on one line')
            if text.count('?') > 1:
                say('C2', n, 'nested or chained ternaries')
            elif top_level(text, '?') and not re.search(r'(?<![=!<>])=(?!=)', text) \
                    and not text.startswith('return'):
                say('C2', n, 'a ternary used to control the flow')
        state['extra'] = 0
        state['ended'] = src.line_of(end)
        while i < len(tokens) and tokens[i][1] <= end:
            i += 1

    empty = blank_lines(src, opens + 1, closes - 1)
    allowed = state['declared'][-1] + 1 if state['declared'] and state['coded'] else 0
    for n in empty:
        if n != allowed:
            say('L6', n, 'an empty line inside a function')
    if allowed and allowed not in empty:
        say('L6', allowed, 'no empty line after the declarations')


def check(path, text, declared):
    src = Source(path, text)
    if not path.endswith(('.c', '.h')):
        src.say('O2', 1, "a source in C is a .c or a .h, and this is a '%s'" % Path(path).suffix)
        return src.found
    functions, statements = file_scope(src)
    check_lines(src)
    check_file(src, functions)
    check_outside(src, statements, functions, declared)
    for f in functions:
        check_function(src, f)
    return src.found


def exceptions():
    """docs/style.md, *Exceptions*: a rule, a path that may hold wildcards and may end in
    :function, and the reason, on a line indented by four."""
    text = (ROOT / 'docs/style.md').read_text(encoding='utf-8')
    listed = []
    for line in text.split('## Exceptions', 1)[1].split('\n'):
        m = re.match(r'    ([A-Z]\d)\s+(\S+)\s+\S', line)
        if m:
            where, _, function = m.group(2).partition(':')
            listed.append([m.group(1), where, function, 0])
    return listed


def ours():
    return sorted({p.relative_to(ROOT).as_posix() for pattern in OURS + ('tests/reference/*',)
                   for p in ROOT.glob(pattern) if p.is_file()})


def prototypes():
    """Every function a header of ours declares: what G5 does not ask to be static."""
    names = set()
    for path in ours():
        if path.endswith('.h'):
            src = Source(path, (ROOT / path).read_text(encoding='utf-8'))
            names.update(re.findall(r'\b([A-Za-z_]\w*)\s*\(', src.code))
    return names


def tracked():
    """O1: nothing a build makes is in the repository. What somebody else built and published
    is told apart by the licence its folder holds. O4: every file and folder of ours that is
    tracked is named in snake_case, but one a tool names, which begins with a dot."""
    listed = subprocess.run(['git', 'ls-files'], cwd=ROOT, capture_output=True, text=True).stdout.split('\n')
    theirs = tuple(path.rsplit('/', 1)[0] + '/' for path in listed
                   if re.search(r'/(?:LICENSE|LICENCE|COPYING)[^/]*\Z', path))
    unwanted = re.compile(r'\.(?:o|obj|a|so|dll|dylib|exe|gch|d|pyc)\Z|~\Z|(?:\A|/)#[^/]*#\Z|(?:\A|/)a\.out\Z')
    ours_only = [path for path in listed if path and not path.startswith(theirs + ('.',))]
    found = [(path, 1, 'O1', 'a compiled, temporary or unnecessary file is tracked', None)
             for path in ours_only if unwanted.search(path)]
    for path in ours_only:
        for part in path.split('/'):
            if not re.match(r'[a-z0-9_]+(?:\.[a-z0-9]+)*\Z', part):
                found.append((path, 1, 'O4', "'%s' is not in snake_case" % part, None))
    return found


# a rule, and C that infringes it and nothing else of what a line shows
PROVES = {
    'F3': 'int a_function_with_a_name_long_enough(void);' + ' ' * 40 + 'int x;\n',
    'F4': 'int f(void)\n{\n' + '    g();\n' * 21 + '}\n',
    'F5': 'int f(int a, int b, int c, int d, int e);\n',
    'F6': 'int f(void)\n{\n    /* why */\n    return (0);\n}\n',
    'L1': 'int f(int a)\n{\n    if (a) return (1);\n    return (0);\n}\n',
    'L2': 'int f(void)\n{\n  return (0);\n}\n',
    'L3': 'int f(int a,int b);\n',
    'L4': 'int f(int a)\n{\n    if (a)\n    {\n        g();\n    }\n    return (0);\n}\n',
    'L5': 'int f(void)\n{\n    int a = 1, b = 2;\n\n    return (a + b);\n}\n',
    'L6': 'int f(void)\n{\n    g();\n\n    return (0);\n}\n',
    'V1': 'typedef int age;\n',
    'V3': 'int f(char* name);\n',
    'C1': 'int f(int a)\n{\n    while (a)\n        for (;;)\n            if (a)\n                g();\n    return (0);\n}\n',
    'C2': 'int f(int a)\n{\n    a ? g() : h();\n    return (0);\n}\n',
    'C3': 'int f(void)\n{\n    goto out;\n    return (0);\n}\n',
    'G4': 'int counter;\n',
    'H3': '#define LOOP_COUNTER 0\n',
    'F2': 'static inline int doIt(void)\n{\n    return (0);\n}\n',
    'G2': 'static inline int f(void)\n{\n    return (0);\n}\nstatic inline int g(void)\n{\n    return (1);\n}\n',
    'G3': '#ifdef A\n#define B(x) (x)\n#endif\n',
    'O3': 'static inline int f(void)\n{\n    return (0);\n}\n\n' * 6,
}
# the same, for what only a source file can infringe
PROVES_IN_C = {
    'G5': 'int f(void)\n{\n    return (0);\n}\n',
    'H1': 'typedef int age_t;\n',
}
CLEAN = ('static int add_them(int first, int second)\n{\n    int sum = first + second;\n\n'
         '    if (sum < 0)\n        return (0);\n    for (int i = 0; i < 2; i++) {\n        sum += i;\n    }\n'
         '    return (sum);\n}\n')


def self_test():
    """Each rule is found in the C that infringes it, and nothing is found in clean C."""
    wrong = []
    for path, proves in (('x.h', PROVES), ('x.c', PROVES_IN_C)):
        for rule, text in proves.items():
            if rule not in {f[2] for f in check(path, text, set())}:
                wrong.append('%s is not found in what infringes it' % rule)
    if 'O4' not in {f[2] for f in check('not-snake.h', '', set())}:
        wrong.append('O4 is not found in what infringes it')
    stray = [f for f in check('x.h', CLEAN, set()) if f[2] not in ('G1', 'H2', 'H1')]
    if stray:
        wrong.append('clean C is reported: %s' % stray)
    return wrong


def main():
    wrong = self_test()
    if wrong:
        sys.exit('FAIL: the checker itself: %s' % '; '.join(wrong))
    everything = '--all' in sys.argv[1:]
    given = [a for a in sys.argv[1:] if not a.startswith('--')]
    paths = given or [p for p in ours() if everything or any(fnmatch.fnmatch(p, held) for held in HELD)]
    declared = prototypes()
    listed = exceptions()
    found = tracked() if not given else []
    for path in paths:
        found += check(path, (ROOT / path).read_text(encoding='utf-8'), declared)
    kept = []
    for path, line, rule, what, function in found:
        excused = False
        for entry in listed:
            if entry[0] == rule and fnmatch.fnmatch(path, entry[1]) and entry[2] in ('', function or ''):
                entry[3] += 1
                excused = True
        if not excused:
            kept.append((path, line, rule, what))
    for entry in listed:
        if not entry[3] and any(fnmatch.fnmatch(p, entry[1]) for p in paths + [f[0] for f in found]):
            kept.append(('docs/style.md', 1, entry[0], "the exception for '%s' excuses nothing" % entry[1]))
    if everything:
        counts = {}
        for _, _, rule, _ in kept:
            counts[rule] = counts.get(rule, 0) + 1
        for rule in sorted(counts):
            print('%s  %5d' % (rule, counts[rule]))
        print('%d files, %d infringements' % (len(paths), len(kept)))
        return
    for path, line, rule, what in sorted(kept):
        print('%s:%d: %s %s' % (path, line, rule, what))
    if kept:
        sys.exit('FAIL: %d infringements of the coding style' % len(kept))
    print('the coding style holds in %d files' % len(paths))


if __name__ == '__main__':
    main()

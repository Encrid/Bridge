"""No crossing per access: the function a built module exports has no call in it.

    nocall.py <wasm-dis> [--enable-...] <module.wasm> <export>

Reads the module as Binaryen prints it and checks the body of the function exported under that
name: it must load or store, and it must not call. An accessor that was merged but not inlined
would be a call here, and one that was never merged would be an import the module still has."""
import re
import subprocess
import sys


def main():
    printer, module, export = sys.argv[1], sys.argv[-2], sys.argv[-1]
    text = subprocess.run([printer] + sys.argv[2:-2] + [module], check=True, capture_output=True,
                          text=True).stdout
    if '(import "acc"' in text:
        sys.exit("FAIL: %s still imports an accessor" % module)
    found = re.search(r'\(export "%s" \(func (\S+)\)\)' % re.escape(export), text)
    if not found:
        sys.exit("FAIL: %s exports no function '%s'" % (module, export))
    start = text.index("(func %s " % found.group(1))
    end = text.find("\n (func ", start + 1)
    body = text[start:end if end != -1 else len(text)]
    calls = len(re.findall(r"\((?:return_)?call(?:_indirect|_ref)?\b", body))
    touches = len(re.findall(r"\([fi](?:32|64)\.(?:load|store)", body))
    print("%s: %d loads and stores, %d calls" % (export, touches, calls))
    if calls or not touches:
        sys.exit("FAIL: '%s' has %d calls and %d loads and stores" % (export, calls, touches))


main()

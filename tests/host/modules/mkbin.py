import sys


def le(v, w):
    return int(v).to_bytes(w, "little")


def make(size):
    return bytearray(size)


def pe_hdr(d, lfanew, sig=True, nsec=0, optsize=0):
    d[0:2] = b"MZ"
    d[0x3c:0x40] = le(lfanew, 4)
    if sig and lfanew + 4 <= len(d):
        d[lfanew:lfanew + 4] = b"PE\0\0"
    if lfanew + 8 <= len(d):
        d[lfanew + 6:lfanew + 8] = le(nsec, 2)
        d[lfanew + 20:lfanew + 22] = le(optsize, 2)


def pe_sec(d, at, name, raw, size):
    d[at:at + 8] = name.ljust(8, b"\0")
    d[at + 8:at + 12] = le(size, 4)
    d[at + 16:at + 20] = le(size, 4)
    d[at + 20:at + 24] = le(raw, 4)


def bins():
    out = {}
    # ---- PE (find_library) ----
    d = make(64)
    pe_hdr(d, 0x7f)
    out["pe_farhdr"] = d                              # pe+6 and pe+24 out of bounds
    d = make(64)
    pe_hdr(d, 57, sig=False)
    out["pe_edge"] = d                                # pe+6 read straddles the end
    d = make(64)
    pe_hdr(d, 0x10, sig=False)
    out["pe_nosig"] = d                               # no PE signature
    d = make(0x40 + 24 + 80)
    pe_hdr(d, 0x40, nsec=1)
    pe_sec(d, 0x40 + 24, b".text", 0x40 + 24, 4)
    out["pe_notbridge"] = d                           # a section, but not .bridge
    d = make(64)
    pe_hdr(d, 0x18, nsec=1)
    out["pe_shorttab"] = d                            # table + 40 runs past the file
    d = make(0x40 + 24 + 80)
    pe_hdr(d, 0x40, nsec=1)
    pe_sec(d, 0x40 + 24, b".bridge", 0x7f7f, 4)
    out["pe_oob"] = d                                 # .bridge data pointer out of bounds
    d = make(0x40 + 24 + 80)
    pe_hdr(d, 0x40, nsec=1)
    pe_sec(d, 0x40 + 24, b".bridge", 0x40 + 24, 0x7f7f)
    out["pe_bigsize"] = d                             # .bridge size out of bounds
    # ---- ELF ----
    d = make(64)
    d[0:6] = b"\x7fELF\x02\x01"
    d[0x28:0x30] = le(0x200, 8)
    out["elf_farshoff"] = d                           # e_shoff past the file
    d = make(64)
    d[0:6] = b"\x7fELF\x02\x01"
    d[0x28:0x30] = le(64, 8)
    d[0x3c:0x3e] = le(5, 2)
    d[0x3e:0x40] = le(1, 2)
    out["elf_badcount"] = d                           # more sections than fit
    out["elf_badstrndx"] = elf_full(shstrndx=9)
    out["elf_nobridge"] = elf_full(name1=9)
    out["elf_oob"] = elf_full(off1=0x7f7f)
    out["elf_bigsize"] = elf_full(size1=0x7f7f)
    out["elf_nameoob"] = elf_full(name1=0x7f7f)
    out["elf_nameedge"] = elf_full(name1=24)
    out["elf_namesoob"] = elf_full(oobstr=True)
    # ---- Mach-O ----
    out["macho_oob"] = macho(at=0x7f7f)
    out["macho_bigsize"] = macho(size=0x7f7f)
    out["macho_notext"] = macho(seg=b"__DATA")
    out["macho_nosec"] = macho(nsects=0)
    out["macho_notseg"] = macho(cmd=0x01)
    out["macho_secoob"] = macho(nsects=99, total=100)
    out["macho_secedge"] = macho(nsects=99, total=184)
    out["macho_cmd0"] = macho(cmdsize0=True)
    out["macho_manycmd"] = macho(ncmds=2, big=True, total=120)
    # ---- wasm ----
    out["wasm_bigsec"] = b"\x00asm\x01\x00\x00\x00\x00\x7f"
    out["wasm_badname"] = b"\x00asm\x01\x00\x00\x00\x00\x05\x0fbrid"
    out["wasm_longleb"] = b"\x00asm\x01\x00\x00\x00\x00\x80\x80\x80\x80\x80\x80"
    out["wasm_cutleb"] = b"\x00asm\x01\x00\x00\x00\x00"
    out["wasm_emptyman"] = wasm_manifest(b"\x00")
    out["wasm_nulfirst"] = wasm_manifest(b"id:x@1\x00\x00abi=1")
    return out


def elf_full(shstrndx=2, name0=0, name1=1, off1=0, size1=4, oobstr=False):
    nsec = 3
    shoff = 64
    strtab = b"\x00.bridge\x00.shstrtab\x00"
    total = shoff + nsec * 64
    d = make(total + len(strtab) + 8)
    d[0:6] = b"\x7fELF\x02\x01"
    d[0x28:0x30] = le(shoff, 8)
    d[0x3c:0x3e] = le(nsec, 2)
    d[0x3e:0x40] = le(shstrndx, 2)
    straddr = total
    if shstrndx < nsec:
        s = shoff + shstrndx * 64
        d[s + 24:s + 32] = le(0x7f7f if oobstr else straddr, 8)
    s0 = shoff + 0 * 64
    d[s0 + 0:s0 + 4] = le(name0, 4)
    s1 = shoff + 1 * 64
    d[s1 + 0:s1 + 4] = le(name1, 4)
    d[s1 + 24:s1 + 32] = le(off1, 8)
    d[s1 + 32:s1 + 40] = le(size1, 8)
    d[straddr:straddr + len(strtab)] = strtab
    return d


def macho(ncmds=1, nsects=1, seg=b"__TEXT", at=0, size=0, cmd=0x19,
          cmdsize0=False, big=False, room=0, total=None):
    body = 72 + nsects * 80
    if total is None:
        total = 32 + body + room
    d = make(total)
    d[0:4] = b"\xcf\xfa\xed\xfe"
    d[16:20] = le(ncmds, 4)
    c = 32
    d[c:c + 4] = le(cmd, 4)
    d[c + 4:c + 8] = le(0 if cmdsize0 else (0x7f7f if big else body), 4)
    d[c + 64:c + 68] = le(nsects, 4)
    if nsects >= 1 and c + 72 + 80 <= total:
        s = c + 72
        d[s:s + 16] = b".bridge".ljust(16, b"\0")
        d[s + 16:s + 32] = seg.ljust(16, b"\0")
        d[s + 40:s + 48] = le(size, 8)
        d[s + 48:s + 52] = le(at, 4)
    return d


def wasm_manifest(content):
    name = b"bridge:manifest"
    body = bytes([len(name)]) + name + content
    return b"\x00asm\x01\x00\x00\x00" + b"\x00" + bytes([len(body)]) + body


def main():
    out, name = sys.argv[1], sys.argv[2]
    open(out, "wb").write(bytes(bins()[name]))


if __name__ == "__main__":
    main()

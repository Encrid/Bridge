wasmtime 48.0.5, its C API, from https://github.com/bytecodealliance/wasmtime, Apache-2.0 with the
LLVM exception (`LICENSE`).

From the official release archives, unpacked and otherwise untouched:

    wasmtime-v48.0.5-x86_64-mingw-c-api.zip
        sha256 7842335b4199bb5cdf3031c0d9084db17ab3d0619829726ed19576d48b5b8ead
    wasmtime-v48.0.5-x86_64-linux-c-api.tar.xz
        sha256 81fffe5fe895c7f8f84a744f4d4d165dda28793d00ee943ee71fcfb6ab5ed923
    wasmtime-v48.0.5-aarch64-macos-c-api.tar.xz
        sha256 92e74e76ef06ae699beec893cf3268d439da4d05dbca020e8f608ca3f0c1e862
    wasmtime-v48.0.5-aarch64-linux-c-api.tar.xz
        sha256 3fd35813cf4a922b968fa3dbd12a300ecea032181a895c4ffe7b1f6f2824fca6
    wasmtime-v48.0.5-x86_64-macos-c-api.tar.xz
        sha256 47789d6be9ae03d4ba0ff13821e1d069bbaa3a8926bcd4b701482c3149539556
    wasmtime-v48.0.5-aarch64-windows-c-api.zip
        sha256 a1b598234681cc04f56f7cb79bcd83f8e001f5545c26ad51363c38102c0065d8

Kept: `include/` (the same in all three archives, line endings aside), the shared library for each
platform, and the Windows import library. Not kept: the static libraries, which are 40 to 80 MB
each, and `min/`.

The Windows one is the `mingw` build, since the library is compiled for `x86_64-windows-gnu`:
`docs/design/build.md`, *Platforms*.

    windows/wasmtime.dll          sha256 ba4c905a80c4d43b71afc16618c0d4142a04b5d17895a6d3194b658e712437ca
    windows/libwasmtime.dll.a     sha256 658a4273fed898c1a51bec469d65c093682b16e76a410204df4dac60dbbddf71
    linux/libwasmtime.so          sha256 b85374b12274b0179209b44e1b6b7467fb3c6a5fb9a70b7e0964053e6d4932ed
    macos/libwasmtime.dylib       sha256 bd837ecc786a413da9e61d352de0e5e0bdd8df8151010ab361123568905a0c0c
    linux_aarch64/libwasmtime.so  sha256 dcf2b22dd0d79de683fa998cb4814c0cfea3c06823c272ee328576f274141524
    macos_x86_64/libwasmtime.dylib
                                  sha256 3ec1929f48209fa817a6969a2d836b0cc5152290f1bce36018437ff2c0890c44
    windows_aarch64/wasmtime.dll  sha256 5b81c517372c56d0a1b8f80188361d2d16d20beaedba80a447b051c37d952cb3
    windows_aarch64/wasmtime.dll.lib
                                  sha256 a85b32a605617c97f44e0e05d47abca0728c317dd77eb86d958fef368f84f8e1

`linux`, `macos` and `windows` are x86-64, arm64 and x86-64, the three the tree began with; a
folder with a processor in its name is the other one. wasmtime publishes one build for Windows on
arm64, by `msvc`, and its import library is what `aarch64-windows-gnu` links: `build.md`,
*Platforms*. The last three were vendored on 2026-10-07.

Vendored on 2026-10-04; the Windows files again, and the macOS one, on 2026-10-05. Nothing here
has been edited.
48 is a long-term release; why that one, and
what the wasm backend does with it, is `docs/seam/backend/impl/wasmtime.md`.

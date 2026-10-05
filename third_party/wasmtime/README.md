wasmtime 48.0.5, its C API, from https://github.com/bytecodealliance/wasmtime, Apache-2.0 with the
LLVM exception (`LICENSE`).

From the official release archives, unpacked and otherwise untouched:

    wasmtime-v48.0.5-x86_64-mingw-c-api.zip
        sha256 7842335b4199bb5cdf3031c0d9084db17ab3d0619829726ed19576d48b5b8ead
    wasmtime-v48.0.5-x86_64-linux-c-api.tar.xz
        sha256 81fffe5fe895c7f8f84a744f4d4d165dda28793d00ee943ee71fcfb6ab5ed923

Kept: `include/` (identical in both archives), the shared library for each platform, and the
Windows import library. Not kept: the static libraries, which are 70 to 80 MB each, and `min/`.

The Windows one is the `mingw` build, since the library is compiled for `x86_64-windows-gnu`:
`docs/design/build.md`, *Platforms*.

    windows/wasmtime.dll          sha256 ba4c905a80c4d43b71afc16618c0d4142a04b5d17895a6d3194b658e712437ca
    windows/libwasmtime.dll.a     sha256 658a4273fed898c1a51bec469d65c093682b16e76a410204df4dac60dbbddf71
    linux/libwasmtime.so       sha256 b85374b12274b0179209b44e1b6b7467fb3c6a5fb9a70b7e0964053e6d4932ed

Vendored on 2026-10-04, and the Windows files again on 2026-10-05. Nothing here has been edited.
48 is a long-term release; why that one, and
what the wasm backend does with it, is `docs/seam/backend/impl/wasmtime.md`.

wasmtime 48.0.5, its C API, from https://github.com/bytecodealliance/wasmtime, Apache-2.0 with the
LLVM exception (`LICENSE`).

From the official release archives, unpacked and otherwise untouched:

    wasmtime-v48.0.5-x86_64-windows-c-api.zip
        sha256 ab2c38453b0caa0d887e206d41752a15208c0dce83f69e8846d7dadd73fb82a5
    wasmtime-v48.0.5-x86_64-linux-c-api.tar.xz
        sha256 81fffe5fe895c7f8f84a744f4d4d165dda28793d00ee943ee71fcfb6ab5ed923

Kept: `include/` (identical in both archives), the shared library for each platform, and the
Windows import library. Not kept: the static libraries, which are 70 to 80 MB each, and `min/`.

    windows/wasmtime.dll       sha256 9efdee0e24e55f50551b192ebcfd67795d8b72d643540bd9a2d829a859c93617
    windows/wasmtime.dll.lib   sha256 68a7fa8dc7adaee7e1aa4c7affca5f49afa1a4a3fc9428700e157ceddc20eb7d
    linux/libwasmtime.so       sha256 b85374b12274b0179209b44e1b6b7467fb3c6a5fb9a70b7e0964053e6d4932ed

Vendored on 2026-10-04. Nothing here has been edited. 48 is a long-term release; why that one, and
what the wasm backend does with it, is `docs/seam/backend/impl/wasmtime.md`.

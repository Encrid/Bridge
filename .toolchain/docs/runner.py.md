# .toolchain/tools/runner.py

`runner.py` writes the commands that give a runner of GitHub what the cases of the project need, for example a display.
The workflows `ci` and `release` give these commands to `bash`, before the step `Fetch`:

```sh
python .toolchain/tools/runner.py --target <target> | bash -e
```

## The needs

`[ci] needs` of the configuration lists the needs of the project: `.toolchain/docs/project.toml.md`, *[ci]*.

| Need | Linux | Windows | macOS |
| --- | --- | --- | --- |
| `display` | Xvfb, a server of X11 with no screen, and the window manager Openbox on it. Then weston, a server of Wayland with no screen. | Nothing: the runner has a desktop. | Nothing: the runner has a desktop. |
| `vulkan` | The packages `libvulkan1` and `mesa-vulkan-drivers`. They give lavapipe, the Vulkan of Mesa that runs on the processor. | Nothing: WARP, the Direct3D 12 of Windows that runs on the processor, is part of the system. | Nothing: Metal is part of the system. |
| `gl` | The packages `libgl1`, `libglx-mesa0` and `libgl1-mesa-dri`. | The file `opengl32.dll` of Mesa, from a release of `mmozeiko/build-mesa`. | Nothing: OpenGL is part of the system. |

`runner.py` writes the commands for the target of `--target`, or for the target of this machine.
`runs_on` of the target gives its system and its processor.
With no need, or with no need on the system, `runner.py` writes nothing. Then `bash` does nothing.

## Linux

The commands do these steps:

1. They run `sudo apt-get update`, then one `sudo apt-get install -y` with the packages of each need, in the order of their names.
2. For `display`, they start Xvfb on the display `:99`, with a screen of 1920 by 1080 pixels and 24 bits.
3. They wait for the socket of the display, then start Openbox on it.
4. They make the folder `$RUNNER_TEMP/wayland`, which only its owner can read, write and open.
5. They start weston in that folder, with its backend `headless`, its renderer `pixman` and the socket `wayland-1`.
6. They wait for that socket.
7. They write `DISPLAY`, `WAYLAND_DISPLAY` and `XDG_RUNTIME_DIR` into `$GITHUB_ENV`. Thus each later step finds the 2 displays.

Xvfb, Openbox and weston run in the background, and send their output to `/dev/null`. They keep running in the later steps of the job.
A wait stops after 10 seconds with a failure, and then the step fails.
weston runs with `--no-config`, so no file of the runner changes it.

## Windows

For `gl`, the commands do these steps:

1. They download `mesa-llvmpipe-<arch>-26.2.5.7z` of `mmozeiko/build-mesa`. `<arch>` is `x64` for `AMD64`, and `arm64` for `ARM64`.
2. They check the SHA-256 digest of the archive. `runner.py` pins the digest of each archive.
3. The `tar.exe` of Windows unpacks `opengl32.dll`. The `tar` of Git cannot unpack a `.7z`.
4. They write `SDL_OPENGL_LIBRARY`, the whole path of the file, into `$GITHUB_ENV`.

Windows finds the `opengl32.dll` of its folder `System32` before the folders of `PATH`. So a library that loads OpenGL needs the whole path.

## Output

`runner.py` writes each command on its own line, with the line end of Linux on each system. `bash` reads a carriage return as a part of a command.
It stops with these messages:

- `[ci] needs: <need> is not a need: display gl vulkan`
- `no machine runs the target <target>`

## Functions

| Function | Result |
| --- | --- |
| `commands(needs, machine)` | Gives the commands of the needs for a machine, as `runs_on` names the machine. |
| `main()` | Reads the options and the needs, and writes the commands. |

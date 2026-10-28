# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import sys

from shared import arguments, config, die, this_target

PACKAGES = {'display': ['openbox', 'weston', 'xvfb'], 'gl': ['libgl1', 'libgl1-mesa-dri', 'libglx-mesa0'],
            'vulkan': ['libvulkan1', 'mesa-vulkan-drivers']}
DISPLAY = ['Xvfb :99 -screen 0 1920x1080x24 > /dev/null 2>&1 &',
           "timeout 10 sh -c 'until [ -S /tmp/.X11-unix/X99 ]; do sleep 0.1; done'",
           'DISPLAY=:99 openbox > /dev/null 2>&1 &',
           'mkdir -p -m 700 "$RUNNER_TEMP/wayland"',
           'XDG_RUNTIME_DIR="$RUNNER_TEMP/wayland" weston --backend=headless --renderer=pixman --socket=wayland-1'
           ' --no-config > /dev/null 2>&1 &',
           'timeout 10 sh -c \'until [ -S "$RUNNER_TEMP/wayland/wayland-1" ]; do sleep 0.1; done\'',
           'echo "DISPLAY=:99" >> "$GITHUB_ENV"',
           'echo "WAYLAND_DISPLAY=wayland-1" >> "$GITHUB_ENV"',
           'echo "XDG_RUNTIME_DIR=$RUNNER_TEMP/wayland" >> "$GITHUB_ENV"']
MESA = {'AMD64': ('x64', 'd1b713ca2a7fba47a50c058099f13c435ddb76a16a422c334e4d787f9755a31d'),
        'ARM64': ('arm64', 'e7df75398d50e07a68bbffa5cff2ece4c85e5f8ef8bda497f560dd04fe60cbbd')}
GL = ['mesa="$(cygpath -u "$RUNNER_TEMP")/mesa"',
      'mkdir -p "$mesa"',
      'curl -fsSL -o "$mesa/mesa.7z" https://github.com/mmozeiko/build-mesa/releases/download/26.2.5/'
      'mesa-llvmpipe-%(arch)s-26.2.5.7z',
      'echo "%(digest)s  $mesa/mesa.7z" | sha256sum -c',
      '/c/Windows/System32/tar.exe -xf "$(cygpath -w "$mesa/mesa.7z")" -C "$(cygpath -w "$mesa")" opengl32.dll',
      'echo "SDL_OPENGL_LIBRARY=$(cygpath -w "$mesa/opengl32.dll")" >> "$GITHUB_ENV"']


def commands(needs, machine):
    system, cpu = machine
    if system == 'Linux':
        packages = sorted(package for need in needs for package in PACKAGES[need])
        installed = ['sudo apt-get update', 'sudo apt-get install -y ' + ' '.join(packages)] if packages else []
        return installed + (DISPLAY if 'display' in needs else [])
    if system == 'Windows' and 'gl' in needs:
        arch, digest = MESA[cpu]
        return [line % {'arch': arch, 'digest': digest} for line in GL]
    return []


def main():
    args = arguments()
    settings = config(args.root)
    target = args.target or this_target(settings['targets'])
    machine = settings['targets'].get(target, {}).get('runs_on') or die('no machine runs the target %s' % target)
    needs = settings.get('ci', {}).get('needs', [])
    for need in needs:
        if need not in PACKAGES:
            die('[ci] needs: %s is not a need: %s' % (need, ' '.join(PACKAGES)))
    sys.stdout.buffer.write(''.join(line + '\n' for line in commands(needs, machine)).encode('utf-8'))


if __name__ == '__main__':
    main()

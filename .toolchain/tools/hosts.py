# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import sys

from shared import arguments, built_by, config, die, hosts, this_target


def main():
    args = arguments('--host')
    settings = config(args.root)
    host = args.host or this_target(settings['targets'])
    if host not in hosts(settings):
        die('%s is not a host of the project' % host)
    sys.stdout.buffer.write(''.join(name + '\n' for name in built_by(settings, host)).encode('utf-8'))


if __name__ == '__main__':
    main()

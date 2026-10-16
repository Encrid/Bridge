# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import os
import subprocess
import sys
from pathlib import Path

at = sys.argv.index('--')
folder, messages, command = Path(sys.argv[1]), sys.argv[2:at], sys.argv[at + 1:]
env = dict(os.environ, GIT_CONFIG_GLOBAL=str(folder / 'none'), GIT_CONFIG_NOSYSTEM='1',
           GIT_AUTHOR_NAME='c23', GIT_AUTHOR_EMAIL='c23@example.com',
           GIT_COMMITTER_NAME='c23', GIT_COMMITTER_EMAIL='c23@example.com')
folder.mkdir(parents=True, exist_ok=True)
subprocess.run(['git', 'init', '-q', str(folder)], env=env, check=True)
for message in messages:
    subprocess.run(['git', '-C', str(folder), 'commit', '-q', '--allow-empty', '-m', message], env=env, check=True)
sys.exit(subprocess.run(command).returncode)

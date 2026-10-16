# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import base64
import datetime
import json
import os
import sys
import urllib.error
import urllib.request

from shared import die

SENTENCE = 'I have read cla.md and I sign it.'
BRANCH, RECORD = 'cla-signatures', 'signatures.json'
ASK = '''Thank you for this pull request.
Before we can merge it, @%s must sign the agreement cla.md at the root of this repository.
The agreement is a licence. You keep the copyright of your work.
The holder can also sell the project under different terms, and these terms can include your contribution.
Read cla.md before you sign it.
To sign it, write a comment that contains only this sentence:

    %s
'''


def decide(author, signed, by=None, said=None):
    if author in signed:
        return 'nothing', True, '%s has signed cla.md' % author
    if by == author and said is not None and said.strip() == SENTENCE:
        return 'record', True, '%s signs cla.md' % author
    return 'ask', False, '%s has not signed cla.md' % author


def call(method, path, body=None):
    request = urllib.request.Request(
        'https://api.github.com/repos/%s%s' % (os.environ['GITHUB_REPOSITORY'], path), method=method,
        data=json.dumps(body).encode() if body is not None else None,
        headers={'Authorization': 'Bearer ' + os.environ['GITHUB_TOKEN'],
                 'Accept': 'application/vnd.github+json', 'X-GitHub-Api-Version': '2022-11-28'})
    try:
        with urllib.request.urlopen(request) as answer:
            return json.loads(answer.read() or b'null')
    except urllib.error.HTTPError as wrong:
        if wrong.code == 404:
            return None
        die('%s %s answered %d: %s' % (method, path, wrong.code, wrong.read().decode('utf-8', 'replace')))


def signatures():
    found = call('GET', '/contents/%s?ref=%s' % (RECORD, BRANCH))
    if found is None:
        return [], None
    return json.loads(base64.b64decode(found['content'])), found['sha']


def record(signed, version, author, number):
    if call('GET', '/git/ref/heads/%s' % BRANCH) is None:
        default = call('GET', '')['default_branch']
        tip = call('GET', '/git/ref/heads/%s' % default)['object']['sha']
        call('POST', '/git/refs', {'ref': 'refs/heads/' + BRANCH, 'sha': tip})
    signed = signed + [{'name': author, 'day': datetime.date.today().isoformat(), 'pull_request': number}]
    body = {'message': 'docs: record the agreement of %s' % author, 'branch': BRANCH,
            'content': base64.b64encode((json.dumps(signed, indent=2) + '\n').encode()).decode()}
    if version:
        body['sha'] = version
    call('PUT', '/contents/' + RECORD, body)


def main():
    event = json.load(open(os.environ['GITHUB_EVENT_PATH'], encoding='utf-8'))
    pull = event.get('pull_request') or call('GET', '/pulls/%d' % event['issue']['number'])
    comment = event.get('comment', {})
    author, number, tip = pull['user']['login'], pull['number'], pull['head']['sha']
    signed, version = signatures()
    what, passes, why = decide(author, [one['name'] for one in signed], comment.get('user', {}).get('login'),
                               comment.get('body'))
    if what == 'record':
        record(signed, version, author, number)
    question = ASK % (author, SENTENCE)
    if what == 'ask' and not any(question in one['body'] for one in call('GET', '/issues/%d/comments?per_page=100' % number) or []):
        call('POST', '/issues/%d/comments' % number, {'body': question})
    call('POST', '/statuses/' + tip, {'state': 'success' if passes else 'failure', 'context': 'cla', 'description': why})
    print(why)


if __name__ == '__main__':
    if sys.argv[1:2] == ['--decide']:
        if len(sys.argv) not in (4, 6):
            die('usage: cla.py --decide <author> <signed> [<by> <said>]')
        what, passes, why = decide(sys.argv[2], [name for name in sys.argv[3].split(',') if name], *sys.argv[4:6])
        print('%s, %s: %s' % (what, 'passes' if passes else 'fails', why))
    else:
        main()

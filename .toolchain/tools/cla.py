# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import base64
import datetime
import hashlib
import json
import os
import sys
import urllib.error
import urllib.request

from shared import die

SENTENCE = 'I have read cla.md and I sign it.'
BRANCH, RECORD, PAGE, TRIES = 'cla-signatures', 'signatures.json', 100, 3
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


def call(method, path, body=None, empty=(404,)):
    request = urllib.request.Request(
        'https://api.github.com/repos/%s%s' % (os.environ['GITHUB_REPOSITORY'], path), method=method,
        data=json.dumps(body).encode() if body is not None else None,
        headers={'Authorization': 'Bearer ' + os.environ['GITHUB_TOKEN'],
                 'Accept': 'application/vnd.github+json', 'X-GitHub-Api-Version': '2022-11-28'})
    try:
        with urllib.request.urlopen(request) as answer:
            return json.loads(answer.read() or b'null')
    except urllib.error.HTTPError as wrong:
        if wrong.code in empty:
            return None
        die('%s %s answered %d: %s' % (method, path, wrong.code, wrong.read().decode('utf-8', 'replace')))


def agreement():
    with open('cla.md', 'rb') as text:
        return hashlib.sha256(text.read()).hexdigest()


def signatures():
    found = call('GET', '/contents/%s?ref=%s' % (RECORD, BRANCH))
    if found is None:
        return [], None
    return json.loads(base64.b64decode(found['content'])), found['sha']


def record(user, digest, number):
    if call('GET', '/git/ref/heads/%s' % BRANCH) is None:
        default = call('GET', '')['default_branch']
        tip = call('GET', '/git/ref/heads/%s' % default)['object']['sha']
        call('POST', '/git/refs', {'ref': 'refs/heads/' + BRANCH, 'sha': tip})
    item = {'name': user['login'], 'id': user['id'], 'cla': digest, 'day': datetime.date.today().isoformat(),
            'pull_request': number}
    for _ in range(TRIES):
        signed, version = signatures()
        body = {'message': 'docs: record the agreement of %s' % user['login'], 'branch': BRANCH,
                'content': base64.b64encode((json.dumps(signed + [item], indent=2) + '\n').encode()).decode()}
        if version:
            body['sha'] = version
        if call('PUT', '/contents/' + RECORD, body, (404, 409)) is not None:
            return
    die('%s changed %d times during the record: run the workflow cla again' % (RECORD, TRIES))


def asked(number, question):
    page = 1
    while True:
        comments = call('GET', '/issues/%d/comments?per_page=%d&page=%d' % (number, PAGE, page)) or []
        if any(question in one['body'] for one in comments):
            return True
        if len(comments) < PAGE:
            return False
        page += 1


def main():
    event = json.load(open(os.environ['GITHUB_EVENT_PATH'], encoding='utf-8'))
    pull = event.get('pull_request') or call('GET', '/pulls/%d' % event['issue']['number'])
    comment = event.get('comment', {})
    user, number, tip = pull['user'], pull['number'], pull['head']['sha']
    digest = agreement()
    signed, _ = signatures()
    names = [user['login']] if any(one.get('id') == user['id'] and one.get('cla') == digest for one in signed) else []
    what, passes, why = decide(user['login'], names, comment.get('user', {}).get('login'), comment.get('body'))
    if what == 'record':
        record(user, digest, number)
    question = ASK % (user['login'], SENTENCE)
    if what == 'ask' and not asked(number, question):
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

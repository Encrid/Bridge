#!/usr/bin/env python3
"""Whether whoever opened a pull request has signed cla.md, and the record of who has.

It is run by .github/workflows/cla.yml when a pull request is opened or changed, and when a
comment is made on one. What it decides:

    the holder         opens a pull request: nothing is asked
    a name on record   has signed: the pull request passes
    anybody else       is asked, once, in a comment, to answer with the sentence below, and
                       the pull request fails until they have
    that sentence      as the whole of a comment, by whoever opened the pull request: their
                       name, the day and the pull request are recorded, and it passes

The record is signatures.json on the branch cla-signatures of the repository, so that it stays
with the project. Nothing of a pull request is ever run: it is read as data, by this file as
it is on the default branch.

    python tools/cla.py                                   as the workflow runs it
    python tools/cla.py --decide AUTHOR HOLDER SIGNED [BY SAID]
                                                          what would be decided, and no more:
                                                          SIGNED is names with commas between
"""
import base64
import datetime
import json
import os
import sys
import urllib.error
import urllib.request

SENTENCE = 'I have read cla.md and I sign it.'
BRANCH, RECORD, MARK = 'cla-signatures', 'signatures.json', '<!-- cla -->'
ASK = """%s

Thank you. Before this can be merged, `cla.md` at the root of the repository has to be signed by
@%s. It is a licence and not a transfer: you keep the copyright of what you wrote. It says
plainly that Bridge is also offered under terms that are paid for, and that a contribution may
be part of that. Please read it.

To sign, answer here with a comment that is this sentence and nothing else:

    %s
"""


def decide(author, holder, signed, by=None, said=None):
    """What is done for a pull request, and whether it passes: nothing, ask or record."""
    if author == holder:
        return 'nothing', True, 'the holder needs no agreement'
    if author in signed:
        return 'nothing', True, '%s has signed cla.md' % author
    if by == author and said is not None and said.strip() == SENTENCE:
        return 'record', True, '%s signs cla.md' % author
    return 'ask', False, '%s has not signed cla.md' % author


def call(method, path, body=None):
    """One request to the service's API for this repository. None when the thing is not there."""
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
        sys.exit('cla: %s %s answered %d: %s' % (method, path, wrong.code, wrong.read().decode('utf-8', 'replace')))


def signatures():
    """Who has signed, and the version of the record that says so."""
    found = call('GET', '/contents/%s?ref=%s' % (RECORD, BRANCH))
    if found is None:
        return [], None
    return json.loads(base64.b64decode(found['content'])), found['sha']


def record(signed, version, author, number):
    """Add a name to the record, making its branch the first time."""
    if call('GET', '/git/ref/heads/%s' % BRANCH) is None:
        default = call('GET', '')['default_branch']
        tip = call('GET', '/git/ref/heads/%s' % default)['object']['sha']
        call('POST', '/git/refs', {'ref': 'refs/heads/' + BRANCH, 'sha': tip})
    signed = signed + [{'name': author, 'day': datetime.date.today().isoformat(), 'pull_request': number}]
    body = {'message': 'docs: %s signs the agreement' % author, 'branch': BRANCH,
            'content': base64.b64encode((json.dumps(signed, indent=2) + '\n').encode()).decode()}
    if version:
        body['sha'] = version
    call('PUT', '/contents/' + RECORD, body)


def main():
    event = json.load(open(os.environ['GITHUB_EVENT_PATH'], encoding='utf-8'))
    by = said = None
    if 'pull_request' in event:
        pull = event['pull_request']
    else:
        if 'pull_request' not in event.get('issue', {}):
            print('cla: a comment that is not on a pull request')
            return
        pull = call('GET', '/pulls/%d' % event['issue']['number'])
        by, said = event['comment']['user']['login'], event['comment']['body']
    author, number, tip = pull['user']['login'], pull['number'], pull['head']['sha']
    signed, version = signatures()
    names = [one['name'] for one in signed]
    what, passes, why = decide(author, os.environ['GITHUB_REPOSITORY_OWNER'], names, by, said)
    if what == 'record':
        record(signed, version, author, number)
    if what == 'ask':
        asked = call('GET', '/issues/%d/comments?per_page=100' % number) or []
        if not any(MARK in comment['body'] for comment in asked):
            call('POST', '/issues/%d/comments' % number, {'body': ASK % (MARK, author, SENTENCE)})
    call('POST', '/statuses/' + tip, {'state': 'success' if passes else 'failure', 'context': 'cla',
                                      'description': why})
    print('cla: %s' % why)


if __name__ == '__main__':
    if sys.argv[1:2] == ['--decide']:
        if len(sys.argv) not in (5, 7):
            sys.exit('usage: cla.py --decide AUTHOR HOLDER SIGNED [BY SAID]')
        what, passes, why = decide(sys.argv[2], sys.argv[3], [n for n in sys.argv[4].split(',') if n],
                                   *sys.argv[5:7])
        print('%s, %s: %s' % (what, 'passes' if passes else 'fails', why))
    else:
        main()

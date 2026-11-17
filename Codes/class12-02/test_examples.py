#!/usr/bin/env python3
"""12주차 2차시 예제 자동 검사: pipe_two · pipetalk · pipeline · fifosrv.
사용법: make && python3 test_examples.py   (Linux 기준, 임시 디렉터리 사용)"""
import os, signal, stat, subprocess, sys, tempfile, time

HERE = os.path.dirname(os.path.abspath(__file__))
PASS = FAIL = 0

def run(args, cwd, timeout=20):
    return subprocess.run(args, cwd=cwd, capture_output=True, text=True, timeout=timeout)

def check(name, cond, detail=''):
    global PASS, FAIL
    if cond:
        PASS += 1; print(f'ok   {name}')
    else:
        FAIL += 1; print(f'FAIL {name} {detail}')

def exe(n): return os.path.join(HERE, n)

def hangs(args, cwd, wait=1.5):
    """프로세스 그룹으로 띄워 wait초 뒤에도 살아 있는지 보고, 그룹째 정리한다."""
    p = subprocess.Popen(args, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                         text=True, start_new_session=True)
    time.sleep(wait)
    alive = p.poll() is None
    try:
        os.killpg(p.pid, signal.SIGTERM)
    except ProcessLookupError:
        pass
    out, err = p.communicate()
    return alive, out, err

with tempfile.TemporaryDirectory() as d:
    # pipe_two
    r = run([exe('pipe_two')], d)
    check('pipe_two prints four fds', r.stdout.startswith('req = ') and 'rep = ' in r.stdout, r.stdout)
    check('pipe_two child gets the question', '[자식] 질문 5바이트: ping' in r.stdout, r.stdout)
    check('pipe_two parent gets the answer', r.returncode == 0 and r.stdout.endswith('[부모] 답 5바이트: pong\n'), r.stdout)

    # pipetalk ask: 질문 셋에 답 셋
    r = run([exe('pipetalk'), 'ask'], d)
    check('pipetalk ask gets three answers', all(f'답:   {s}\n' in r.stdout for s in ('3', '70', '1100')), r.stdout)
    check('pipetalk ask child exits on EOF', '자식 종료 코드 0' in r.stdout and r.returncode == 0, r.stdout)

    # pipetalk both · mute: 서로 기다려 끝나지 않는다
    alive, out, _ = hangs([exe('pipetalk'), 'both'], d)
    check('pipetalk both hangs (both sides read first)', alive)
    alive, out, _ = hangs([exe('pipetalk'), 'mute'], d)
    check('pipetalk mute hangs (answer stays in the stdio buffer)', alive)

    # pipetalk flood · fix
    r = run([exe('pipetalk'), 'flood', '20000'], d)
    check('pipetalk flood small returns everything', '돌려받은 양 = 20000바이트' in r.stdout and r.returncode == 0, r.stdout)
    alive, out, err = hangs([exe('pipetalk'), 'flood', '1000000'], d)
    check('pipetalk flood 1000000 hangs (both pipes full)', alive and '돌려받은 양' not in out, out)
    if sys.platform == 'linux':
        check('pipetalk flood stops after 131072 on Linux',
              '[보냄] 131072바이트' in err and '[보냄] 163840바이트' not in err, err)
    r = run([exe('pipetalk'), 'fix', '1000000'], d)
    check('pipetalk fix 1000000 returns everything', '돌려받은 양 = 1000000바이트' in r.stdout and r.returncode == 0, r.stdout)
    r = run([exe('pipetalk')], d)
    check('pipetalk usage -> 2', r.returncode == 2)

    # pipeline
    for n in ('a.txt', 'b.txt', 'c.txt'): open(os.path.join(d, n), 'w').close()
    r = run([exe('pipeline'), 'ls', '--', 'sort', '-r', '--', 'head', '-1'], d)
    check('pipeline 3 stages: ls | sort -r | head -1', r.stdout == 'c.txt\n' and r.returncode == 0, r.stdout)
    check('pipeline reports every stage', all(s in r.stderr for s in ('[1] ls: 종료 코드 0', '[2] sort: 종료 코드 0', '[3] head: 종료 코드 0')), r.stderr)
    r = run([exe('pipeline'), 'ls', '--', 'cat', '--', 'cat', '--', 'cat', '--', 'wc', '-l'], d)
    check('pipeline 5 stages count 3', r.stdout.strip() == '3', r.stdout)
    r = run([exe('pipeline'), '-v', 'ls', '--', 'cat', '--', 'wc', '-l'], d)
    check('pipeline -v prints the ledger', r.stderr.count('[부모]') == 3 and '입력 -1' in r.stderr and '출력 -1' in r.stderr, r.stderr)
    r = run([exe('pipeline'), 'yes', '--', 'head', '-2', '--', 'wc', '-l'], d)
    check('pipeline yes | head -2 | wc -l: yes dies of SIGPIPE', r.stdout.strip() == '2' and '[1] yes: 시그널 13' in r.stderr, r.stderr)
    r = run([exe('pipeline'), 'ls', '--', 'cat', '--', 'grep', 'zzz'], d)
    check('pipeline exit status is the last stage (grep 1)', r.returncode == 1, str(r.returncode))
    r = run([exe('pipeline'), 'false', '--', 'cat', '--', 'true'], d)
    check('pipeline ignores earlier failures (false | cat | true -> 0)', r.returncode == 0 and '[1] false: 종료 코드 1' in r.stderr, r.stderr)
    r = run([exe('pipeline'), 'ls', '--', 'nosuch-cmd', '--', 'wc', '-l'], d)
    check('pipeline missing command -> 127 reported', '[2] nosuch-cmd: 종료 코드 127' in r.stderr and r.stdout.strip() == '0', r.stderr)
    t = time.time()
    r = run([exe('pipeline'), 'sleep', '1', '--', 'sleep', '1', '--', 'sleep', '1'], d)
    check('pipeline stages run at the same time (3 x sleep 1 < 2s)', time.time() - t < 2.0 and r.returncode == 0)
    r = run([exe('pipeline'), 'ls'], d)
    check('pipeline usage (one stage) -> 2', r.returncode == 2)
    r = run([exe('pipeline'), 'ls', '--', '--', 'wc'], d)
    check('pipeline usage (empty stage) -> 2', r.returncode == 2)
    alive, out, err = hangs([exe('pipeline'), '-k', 'ls', '--', 'cat', '--', 'wc', '-l'], d)
    check('pipeline -k hangs (later stages never get EOF)', alive and '[1] ls: 종료 코드 0' in err, err)

    # fifosrv
    fifo = os.path.join(d, 'calc')
    srv = subprocess.Popen([exe('fifosrv'), 'serve', fifo], cwd=d, stdout=subprocess.PIPE, text=True)
    for _ in range(50):
        if os.path.exists(fifo): break
        time.sleep(0.05)
    check('fifosrv serve creates a FIFO', os.path.exists(fifo) and stat.S_ISFIFO(os.stat(fifo).st_mode))
    r = run([exe('fifosrv'), 'ask', fifo, '3', '4'], d)
    check('fifosrv ask 3 4 -> 7', '3 + 4 = 7' in r.stdout and r.returncode == 0, r.stdout)
    r = run([exe('fifosrv'), 'ask', fifo, '10', '-25'], d)
    check('fifosrv keeps serving after a client leaves', '10 + -25 = -15' in r.stdout, r.stdout)
    r = run([exe('fifosrv'), 'burst', fifo, '30'], d)
    check('fifosrv burst 30: every client gets its own answer', '30개를 보내 30개가' in r.stdout and r.returncode == 0, r.stdout)
    check('fifosrv clients remove their reply FIFOs', os.listdir(d).count('calc') == 1 and not [n for n in os.listdir(d) if n.startswith('calc.')], str(os.listdir(d)))
    check('fifosrv server is still alive', srv.poll() is None)
    r = run([exe('fifosrv'), 'stop', fifo], d)
    out = srv.communicate(timeout=10)[0]
    check('fifosrv stop ends the server', '요청 32개를 처리하고 끝낸다' in out and srv.returncode == 0, out[-200:])
    check('fifosrv server unlinked the FIFO', not os.path.exists(fifo))
    r = run([exe('fifosrv'), 'ask', fifo, '1', '2'], d)
    check('fifosrv ask without a server -> 1', r.returncode == 1 and 'No such file' in r.stderr, r.stderr)
    check('fifosrv failed ask leaves no reply FIFO', not [n for n in os.listdir(d) if n.startswith('calc')], str(os.listdir(d)))
    r = run([exe('fifosrv')], d)
    check('fifosrv usage -> 2', r.returncode == 2)

print(f'\n{PASS} passed, {FAIL} failed')
sys.exit(1 if FAIL else 0)

#!/usr/bin/env python3
"""13주차 1차시 예제 자동 검사: isolated · pmq · mqserver/mqclient.
사용법: make && python3 test_examples.py   (POSIX 메시지 큐는 Linux 전용)"""
import os, re, signal, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
PASS = FAIL = 0
LINUX = sys.platform == 'linux'

def run(*args, timeout=20):
    return subprocess.run([os.path.join(HERE, args[0]), *args[1:]], capture_output=True, text=True, timeout=timeout)

def check(name, cond, detail=''):
    global PASS, FAIL
    if cond:
        PASS += 1; print(f'ok   {name}')
    else:
        FAIL += 1; print(f'FAIL {name} {detail}')

# isolated
r = run('isolated')
m = re.findall(r'counter=(\d+) \((0x[0-9a-f]+)\)  \*heap=(\d+) \((0x[0-9a-f]+)\)', r.stdout)
check('isolated: child sees 101/201, parent still 100/200', [x[0] + '/' + x[2] for x in m] == ['101/201', '100/200'], r.stdout)
check('isolated: same virtual addresses in both', len(m) == 2 and m[0][1] == m[1][1] and m[0][3] == m[1][3], r.stdout)
check('isolated: only the exit status 42 comes back', '종료 상태 42' in r.stdout)


if LINUX:
    Q = f'/w13test.{os.getpid()}'
    run('pmq', 'unlink', Q)
    r = run('pmq', 'create', Q)
    check('pmq create', r.returncode == 0 and 'maxmsg=10' in r.stdout, r.stderr)
    check('pmq create twice -> EEXIST', 'File exists' in run('pmq', 'create', Q).stderr)
    check('pmq queue is visible in /dev/mqueue', os.path.exists('/dev/mqueue' + Q))
    for p, s in [('1', 'low-a'), ('5', 'urgent'), ('1', 'low-b')]:
        run('pmq', 'send', Q, p, s)
    check('pmq stat counts 3 messages', 'curmsgs=3' in run('pmq', 'stat', Q).stdout)
    out = [run('pmq', 'recv', Q).stdout for _ in range(3)]
    check('pmq recv: highest priority first, FIFO within a priority',
          ['urgent' in out[0], 'low-a' in out[1], 'low-b' in out[2]] == [True] * 3, ''.join(out))
    check('pmq recv keeps message boundaries (len=6)', 'len=6' in out[0], out[0])
    r = run('pmq', 'recv', Q, '-n')
    check('pmq recv -n on an empty queue -> EAGAIN', r.returncode == 1 and 'temporarily unavailable' in r.stderr, r.stderr)
    check('pmq send longer than msgsize -> EMSGSIZE', 'Message too long' in run('pmq', 'send', Q, '0', 'x' * 200).stderr)
    p = subprocess.Popen([os.path.join(HERE, 'pmq'), 'recv', Q], stdout=subprocess.PIPE, text=True)
    time.sleep(0.3)
    check('pmq recv blocks on an empty queue', p.poll() is None)
    run('pmq', 'send', Q, '2', 'wake')
    check('pmq recv wakes up when a message arrives', 'wake' in p.communicate(timeout=5)[0])
    check('pmq unlink', run('pmq', 'unlink', Q).returncode == 0 and not os.path.exists('/dev/mqueue' + Q))
    check('pmq open of a missing queue -> ENOENT', 'No such file' in run('pmq', 'send', Q, '0', 'x').stderr)
    check('pmq name without a slash -> EINVAL', 'Invalid argument' in run('pmq', 'create', 'noslash').stderr)
    check('pmq maxmsg above msg_max -> EINVAL', 'Invalid argument' in run('pmq', 'create', Q, '100000', '128').stderr)
    check('pmq usage -> 2', run('pmq').returncode == 2)

    # mqserver / mqclient
    r = run('mqclient', 'hello')
    check('mqclient without a server -> exit 1', r.returncode == 1 and '서버가 없다' in r.stderr, r.stderr)
    srv = subprocess.Popen([os.path.join(HERE, 'mqserver')], stderr=subprocess.PIPE, text=True)
    time.sleep(0.4)
    r = run('mqclient', 'hello')
    check('mqclient hello -> HELLO', 'hello → HELLO' in r.stdout, r.stdout + r.stderr)
    ps = [subprocess.Popen([os.path.join(HERE, 'mqclient'), w], stdout=subprocess.PIPE, text=True) for w in ('apple', 'banana', 'cherry')]
    outs = [p.communicate(timeout=10)[0] for p in ps]
    check('three concurrent clients each get their own reply',
          all(w in o and w.upper() in o for w, o in zip(('apple', 'banana', 'cherry'), outs)), ''.join(outs))
    check('clients remove their reply queues', [f for f in os.listdir('/dev/mqueue') if f.startswith('w13_rep.')] == [])
    srv.send_signal(signal.SIGSTOP)
    t = time.time(); r = run('mqclient', 'late'); dt = time.time() - t
    check('stopped server -> client times out after about 3 s', r.returncode == 1 and 'timed out' in r.stderr and 2.5 < dt < 4.5, f'{dt:.1f} {r.stderr}')
    srv.send_signal(signal.SIGCONT)
    time.sleep(0.3)
    check('mqclient quit', run('mqclient', 'quit').returncode == 0)
    err = srv.communicate(timeout=5)[1]
    check('server logs 4 replies and one missing reply queue', err.count('→') == 4 and '응답 큐 없음' in err, err)
    check('server removes its queue on quit', srv.returncode == 0 and not os.path.exists('/dev/mqueue/w13_req'))
    srv = subprocess.Popen([os.path.join(HERE, 'mqserver')], stderr=subprocess.PIPE, text=True)
    time.sleep(0.4); srv.send_signal(signal.SIGINT)
    err = srv.communicate(timeout=5)[1]
    check('server cleans up on SIGINT', srv.returncode == 0 and '지움' in err and not os.path.exists('/dev/mqueue/w13_req'), err)
else:
    print('skip pmq · mqserver · mqclient (POSIX 메시지 큐는 Linux 전용)')

print(f'\n{PASS} passed, {FAIL} failed')
sys.exit(1 if FAIL else 0)

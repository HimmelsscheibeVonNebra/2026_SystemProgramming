#!/usr/bin/env python3
"""12주차 1차시 예제 자동 검사: pipe_basic · pipemsg · pipe2 · pipeio.
사용법: make && python3 test_examples.py   (Linux 기준, 임시 디렉터리 사용)"""
import os, subprocess, sys, tempfile, time

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

with tempfile.TemporaryDirectory() as d:
    # pipe_basic
    r = run([exe('pipe_basic')], d)
    check('pipe_basic echoes through the pipe', r.returncode == 0 and r.stdout.endswith('hello pipe\n'), repr(r.stdout))
    check('pipe_basic prints two fds', 'fd[0] = ' in r.stdout and 'fd[1] = ' in r.stdout, r.stdout)

    # pipemsg ok: 두 줄을 읽고 EOF를 본다
    r = run([exe('pipemsg'), 'ok'], d)
    check('pipemsg ok child reads both lines', r.stdout.count('[자식] read 15바이트') == 2, r.stdout)
    check('pipemsg ok child sees EOF', 'read 0 → EOF' in r.stdout and '자식 종료 코드 0' in r.stdout, r.stdout)

    # pipemsg leak: 자식이 쓰기 끝을 쥐고 있어 끝나지 않는다
    p = subprocess.Popen([exe('pipemsg'), 'leak'], cwd=d, stdout=subprocess.PIPE, text=True, start_new_session=True)
    time.sleep(1.5)
    check('pipemsg leak is still running after 1.5 s', p.poll() is None)
    os.killpg(p.pid, 15); out = p.communicate()[0]
    check('pipemsg leak never printed EOF', 'EOF' not in out and '두 번째 줄' in out, out)

    # sigpipe · epipe · cap
    r = run([exe('pipemsg'), 'sigpipe'], d)
    check('pipemsg sigpipe killed by SIGPIPE', r.returncode == -13, str(r.returncode))
    r = run([exe('pipemsg'), 'epipe'], d)
    check('pipemsg epipe -> write fails with EPIPE', r.returncode == 1 and 'Broken pipe' in r.stdout, r.stdout)
    r = run([exe('pipemsg'), 'cap'], d)
    check('pipemsg cap reports a full pipe', '가득 찰 때까지 쓴 양 = ' in r.stdout and 'PIPE_BUF' in r.stdout, r.stdout)
    if sys.platform == 'linux':
        check('pipemsg cap is 65536 on Linux', '= 65536바이트' in r.stdout, r.stdout)
    r = run([exe('pipemsg')], d)
    check('pipemsg usage -> 2', r.returncode == 2)

    # pipe2
    for n in ('a.txt', 'b.txt', 'c.txt'): open(os.path.join(d, n), 'w').close()
    r = run([exe('pipe2'), 'ls', '--', 'wc', '-l'], d)
    check('pipe2 ls -- wc -l counts 3', r.stdout.strip() == '3' and r.returncode == 0, r.stdout)
    check('pipe2 reports both children', 'ls: 종료 코드 0' in r.stderr and 'wc: 종료 코드 0' in r.stderr, r.stderr)
    r = run([exe('pipe2'), '-o', 'out.txt', 'ls', '--', 'grep', 'b'], d)
    check('pipe2 -o writes the file', r.stdout == '' and open(os.path.join(d, 'out.txt')).read() == 'b.txt\n', r.stdout)
    r = run([exe('pipe2'), 'yes', '--', 'head', '-2'], d)
    check('pipe2 yes -- head: yes dies of SIGPIPE', r.stdout == 'y\ny\n' and 'yes: 시그널 13' in r.stderr, r.stderr)
    r = run([exe('pipe2'), 'ls', '--', 'grep', 'zzz'], d)
    check('pipe2 exit status is the last command (grep 1)', r.returncode == 1, str(r.returncode))
    r = run([exe('pipe2'), 'nosuch-cmd', '--', 'wc', '-l'], d)
    check('pipe2 missing command -> 127 reported', 'nosuch-cmd: 종료 코드 127' in r.stderr and r.stdout.strip() == '0', r.stderr)
    r = run([exe('pipe2'), 'ls'], d)
    check('pipe2 usage -> 2', r.returncode == 2)
    p = subprocess.Popen([exe('pipe2'), '-k', 'ls', '--', 'wc', '-l'], cwd=d, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, start_new_session=True)
    time.sleep(1.5)
    check('pipe2 -k hangs (wc never gets EOF)', p.poll() is None)
    os.killpg(p.pid, 15); p.communicate()

    # pipeio popen
    r = run([exe('pipeio'), 'popen', 'ls | sort'], d)
    check('pipeio popen numbers the lines', ' 1: a.txt' in r.stdout and '줄 수 4, 명령의 종료 코드 0' in r.stdout, r.stdout)
    r = run([exe('pipeio'), 'popen', 'exit 3'], d)
    check('pipeio popen returns the command status', '줄 수 0, 명령의 종료 코드 3' in r.stdout, r.stdout)

    # pipeio recv / send
    fifo = os.path.join(d, 'chan')
    p = subprocess.Popen([exe('pipeio'), 'recv', fifo], cwd=d, stdout=subprocess.PIPE, text=True)
    for _ in range(50):
        if os.path.exists(fifo): break
        time.sleep(0.05)
    import stat
    check('pipeio recv creates a FIFO', os.path.exists(fifo) and stat.S_ISFIFO(os.stat(fifo).st_mode))
    time.sleep(0.3)
    check('pipeio recv blocks in open until a writer comes', p.poll() is None)
    r = run([exe('pipeio'), 'send', fifo, 'hello fifo'], d)
    out = p.communicate(timeout=10)[0]
    check('pipeio recv got the line and EOF', '11바이트: hello fifo' in out and 'EOF' in out, out)
    check('pipeio recv unlinked the FIFO', not os.path.exists(fifo))
    r = run([exe('pipeio'), 'send', os.path.join(d, 'nope'), 'x'], d)
    check('pipeio send to a missing FIFO -> 1', r.returncode == 1 and 'No such file' in r.stderr, r.stderr)

print(f'\n{PASS} passed, {FAIL} failed')
sys.exit(1 if FAIL else 0)

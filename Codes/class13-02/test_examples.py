#!/usr/bin/env python3
"""13주차 2차시 예제 자동 검사: shmhello · shmrace · semcount · ringbuf · thcount · thqueue · thlimit.
사용법: make && python3 test_examples.py   (프로세스 사이 sem_init은 Linux 전용)"""
import os, re, signal, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
PASS = FAIL = 0
LINUX = sys.platform == 'linux'
exe = lambda n: os.path.join(HERE, n)

def run(*args, timeout=30):
    return subprocess.run([exe(args[0]), *args[1:]], capture_output=True, text=True, timeout=timeout)

def bg(*args):
    return subprocess.Popen([exe(args[0]), *args[1:]], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

def check(name, cond, detail=''):
    global PASS, FAIL
    if cond:
        PASS += 1; print(f'ok   {name}')
    else:
        FAIL += 1; print(f'FAIL {name} {detail}')

def count():
    return int(run('shmrace', 'show').stdout.split('=')[1])

# thqueue · thcount mutex (스레드: 어디서나)
r = run('thqueue', '2', '1', '5')
check('thqueue: 2 producers, 1 consumer, sum 1520', r.returncode == 0 and '합 = 1520 (기대 1520)' in r.stdout, r.stdout)
nums = sorted(int(x) for x in re.findall(r'꺼냄 (\d+)', r.stdout))
check('thqueue: every item exactly once', nums == [100, 101, 102, 103, 104, 200, 201, 202, 203, 204], str(nums))
r = run('thqueue', '4', '2', '50000')
check('thqueue: 4 producers, 2 consumers, 200000 items', r.returncode == 0 and '(기대 ' + r.stdout.split('= ')[1].split(' ')[0] + ')' in r.stdout, r.stdout)
r = run('thqueue', '1', '1', '30', '5')
check('thqueue: a slow consumer makes the producer wait on a full buffer', re.search(r'가득 차서 기다림 [1-9]', r.stdout) is not None, r.stdout)
check('thqueue usage -> 2', run('thqueue').returncode == 2)
check('thcount mutex: four threads are exact', 'count = 4000000 (기대 4000000)' in run('thcount', 'mutex').stdout)
check('thcount usage -> 2', run('thcount', 'bogus').returncode == 2)

if LINUX:
    # shmhello
    run('shmhello', 'rm')
    check('shmhello read before write -> ENOENT', 'No such file' in run('shmhello', 'read').stderr)
    w = run('shmhello', 'write', 'hello shm'); r = run('shmhello', 'read')
    check('shmhello: another process reads the text', '"hello shm"' in r.stdout, r.stdout)
    check('shmhello: object is a 4096-byte file in /dev/shm', os.path.getsize('/dev/shm/w13_hello') == 4096)
    check('shmhello: the bytes are visible through the file', open('/dev/shm/w13_hello', 'rb').read(9) == b'hello shm')
    check('shmhello rm', run('shmhello', 'rm').returncode == 0 and not os.path.exists('/dev/shm/w13_hello'))
    check('shmhello usage -> 2', run('shmhello').returncode == 2)

    # shmrace
    run('shmrace', 'rm')
    check('shmrace add before init -> ENOENT', 'No such file' in run('shmrace', 'add', '1').stderr)
    check('shmrace init', 'count = 0' in run('shmrace', 'init').stdout)
    check('shmrace init twice -> EEXIST', 'File exists' in run('shmrace', 'init').stderr)
    run('shmrace', 'add', '1000000')
    check('shmrace: one process alone is exact', count() == 1000000)
    lost = False
    for _ in range(5):          # 경쟁은 확률적이다: 다섯 번 안에 한 번은 잃는다
        base = count(); ps = [bg('shmrace', 'add', '2000000') for _ in range(4)]
        [p.wait() for p in ps]
        if count() - base < 8000000: lost = True; break
    if (os.cpu_count() or 1) > 1:
        check('shmrace: four processes without a lock lose updates', lost)
    run('shmrace', 'rm')

    # semcount
    run('shmrace', 'init')
    ps = [bg('semcount', 'add', '500000') for _ in range(4)]
    [p.wait() for p in ps]
    check('semcount: four processes with the semaphore are exact', count() == 2000000, str(count()))
    check('semcount value is 1 when idle', '= 1' in run('semcount', 'value').stdout)
    h = bg('semcount', 'hold', '30'); time.sleep(0.4)
    check('semcount value is 0 while held', '= 0' in run('semcount', 'value').stdout)
    t = time.time(); r = run('semcount', 'try'); dt = time.time() - t
    check('semcount try times out after about 2 s', r.returncode == 1 and 'timed out' in r.stderr and 1.5 < dt < 3.5, f'{dt:.1f}')
    h.send_signal(signal.SIGKILL); h.wait()
    check('killed holder leaves the semaphore locked', '= 0' in run('semcount', 'value').stdout)
    run('shmrace', 'rm'); run('shmrace', 'init')
    h = bg('semcount', 'hold', '1'); time.sleep(0.3)
    r = run('semcount', 'try')
    check('semcount try succeeds when the holder releases in time', r.returncode == 0 and 'count = 1' in r.stdout, r.stdout + r.stderr)
    h.wait(); run('shmrace', 'rm')

    # ringbuf
    run('ringbuf', 'rm')
    check('ringbuf init', '8칸' in run('ringbuf', 'init').stdout)
    check('ringbuf stat: 8 free, 0 used', '빈 칸 8 · 든 칸 0' in run('ringbuf', 'stat').stdout)
    p = bg('ringbuf', 'put', '1', '12'); time.sleep(0.5)
    check('producer blocks when the 8 slots are full', p.poll() is None and '빈 칸 0 · 든 칸 8' in run('ringbuf', 'stat').stdout)
    r = run('ringbuf', 'get', '12')
    nums = [int(x) for x in re.findall(r'꺼냄 (\d+)', r.stdout)]
    check('consumer gets 1..12 in order, sum 78', nums == list(range(1, 13)) and '합 = 78' in r.stdout, r.stdout)
    check('producer finishes once the consumer drains', p.wait(timeout=5) == 0)
    c = bg('ringbuf', 'get', '3'); time.sleep(0.3)
    check('consumer blocks on an empty buffer', c.poll() is None)
    run('ringbuf', 'put', '7', '3')
    check('consumer wakes up when items arrive', '합 = 24' in c.communicate(timeout=5)[0])
    c1 = bg('ringbuf', 'get', '1000'); c2 = bg('ringbuf', 'get', '1000')
    ps = [bg('ringbuf', 'put', str(s), '500') for s in (0, 10000, 20000, 30000)]
    o1, o2 = c1.communicate(timeout=30)[0], c2.communicate(timeout=30)[0]
    [p.wait() for p in ps]
    got = sorted(int(x) for x in re.findall(r'꺼냄 (\d+)', o1 + o2))
    want = sorted(s + i for s in (0, 10000, 20000, 30000) for i in range(500))
    check('4 producers, 2 consumers: every item exactly once', got == want, f'{len(got)}')
    check('ringbuf is empty afterwards', '빈 칸 8 · 든 칸 0' in run('ringbuf', 'stat').stdout)
    check('ringbuf rm', run('ringbuf', 'rm').returncode == 0)
    check('ringbuf usage -> 2', run('ringbuf').returncode == 2)
    # thcount none · sem, thlimit (스레드 + 세마포어)
    lost = any('count = 8000000' not in run('thcount', 'none', '4', '2000000').stdout for _ in range(5))
    if (os.cpu_count() or 1) > 1:
        check('thcount none: four threads without a lock lose updates', lost)
    check('thcount sem: four threads are exact', 'count = 4000000 (기대 4000000)' in run('thcount', 'sem').stdout)
    r = run('thlimit', '6', '2')
    check('thlimit: never more than K threads inside', '가장 많을 때 안에 2개 (한도 2)' in r.stdout and '안에 3개' not in r.stdout, r.stdout)
    check('thlimit: all six threads get in and out', r.stdout.count('들어감') == 6 and r.stdout.count('나옴') == 6)
    check('thlimit usage -> 2', run('thlimit').returncode == 2)
else:
    print('skip shmhello · shmrace · semcount · ringbuf · thcount sem · thlimit (Linux 전용 검사)')

print(f'\n{PASS} passed, {FAIL} failed')
sys.exit(1 if FAIL else 0)

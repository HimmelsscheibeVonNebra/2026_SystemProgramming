#!/usr/bin/env python3
"""11주차 1차시 예제 자동 검사: mapcat · mapmode · countnl.
사용법: make && python3 test_examples.py   (Linux 기준, 임시 디렉터리 사용)"""
import os, re, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
PASS = FAIL = 0

def run(args, cwd):
    return subprocess.run(args, cwd=cwd, capture_output=True, text=True, timeout=60)

def check(name, cond, detail=''):
    global PASS, FAIL
    if cond:
        PASS += 1; print(f'ok   {name}')
    else:
        FAIL += 1; print(f'FAIL {name} {detail}')

def exe(n): return os.path.join(HERE, n)

with tempfile.TemporaryDirectory() as d:
    hello = os.path.join(d, 'hello.txt')
    with open(hello, 'w') as f: f.write('hello mmap\nsecond line\n')
    open(os.path.join(d, 'empty.txt'), 'w').close()

    # mapcat
    r = run([exe('mapcat'), 'hello.txt'], d)
    check('mapcat prints file', r.returncode == 0 and r.stdout == 'hello mmap\nsecond line\n', repr(r.stdout))
    r = run([exe('mapcat'), 'empty.txt'], d)
    check('mapcat empty file -> 0, no output', r.returncode == 0 and r.stdout == '' and 'empty' in r.stderr)
    r = run([exe('mapcat'), 'nope.txt'], d)
    check('mapcat missing file -> 1', r.returncode == 1 and 'No such file' in r.stderr)
    r = run([exe('mapcat'), d], d)
    check('mapcat directory -> 1', r.returncode == 1 and 'not a regular file' in r.stderr)
    r = run([exe('mapcat')], d)
    check('mapcat usage -> 2', r.returncode == 2)

    # mapmode private / shared
    for mode, want in (('private', 'hello mmap'), ('shared', 'HELLO MMAP')):
        p = os.path.join(d, f'{mode}.txt')
        with open(p, 'w') as f: f.write('hello mmap\nsecond line\n')
        r = run([exe('mapmode'), mode, p], d)
        check(f'mapmode {mode} mapping uppercased', 'mapping:       HELLO MMAP' in r.stdout, r.stdout)
        check(f'mapmode {mode} file shows {want!r}', f'file (pread):  {want}' in r.stdout, r.stdout)
        check(f'mapmode {mode} second line untouched', open(p).read().split('\n')[1] == 'second line')

    # mapmode anon
    r = run([exe('mapmode'), 'anon'], d)
    check('mapmode anon child sees 42/42', 'child : global=42 shared=42' in r.stdout, r.stdout)
    check('mapmode anon parent global=0 shared=42', 'parent: global=0 shared=42' in r.stdout, r.stdout)

    # mapmode race: 자식 1개는 손실 0, 여러 개면 actual <= expected
    r = run([exe('mapmode'), 'race', '1', '200000'], d)
    check('mapmode race 1 -> lost 0', 'lost 0' in r.stdout, r.stdout)
    r = run([exe('mapmode'), 'race', '4', '1000000'], d)
    m = re.search(r'expected (\d+)\s+actual (\d+)\s+lost (-?\d+)', r.stdout)
    check('mapmode race 4 -> actual <= expected', bool(m) and int(m.group(2)) <= int(m.group(1)), r.stdout)
    if m and int(m.group(3)) == 0:
        print('note mapmode race 4: 이번 실행은 손실 0 (CPU가 1개이거나 운이 좋은 경우)')

    # countnl
    big = os.path.join(d, 'lines.txt')
    with open(big, 'w') as f:
        for i in range(200000): f.write(f'{i}\n')
    outs = {}
    for args in (['read'], ['mmap'], ['mmap', 'seq']):
        r = run([exe('countnl'), args[0], big] + args[1:], d)
        m = re.search(r'lines=(\d+).*minflt=(\d+)\s+majflt=(\d+)', r.stdout)
        outs[' '.join(args)] = m
        check(f'countnl {" ".join(args)} counts 200000', bool(m) and m.group(1) == '200000', r.stdout)
    check('countnl mmap has more minor faults than read',
          all(outs.values()) and int(outs['mmap'].group(2)) > int(outs['read'].group(2)))
    r = run([exe('countnl'), 'drop', big], d)
    check('countnl drop succeeds', r.returncode == 0 and 'dropped' in r.stdout, r.stderr)
    e = os.path.join(d, 'empty.txt')
    r = run([exe('countnl'), 'mmap', e], d)
    check('countnl mmap empty -> lines=0', 'lines=0' in r.stdout, r.stdout)

print(f'\n{PASS} passed, {FAIL} failed')
sys.exit(1 if FAIL else 0)

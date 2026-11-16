#!/usr/bin/env python3
"""10주차 3차시 예제 자동 검사: forkpool · sigctl.
사용법: make && python3 test_examples.py   (Linux 기준)"""
import os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
PASS = FAIL = 0

def run(*args):
    return subprocess.run([os.path.join(HERE, args[0])] + list(args[1:]),
                          capture_output=True, text=True, timeout=30)

def check(name, cond, detail=''):
    global PASS, FAIL
    if cond: PASS += 1; print(f'ok   {name}')
    else:    FAIL += 1; print(f'FAIL {name} {detail}')

def total_ms(out):
    m = re.search(r'(\d+) ms  all (\d+) jobs finished, (\d+) failed', out)
    return (int(m.group(1)), int(m.group(2)), int(m.group(3))) if m else None

# forkpool
r = run('forkpool', '6', '2')
out = r.stdout
check('forkpool 6 2 exits 0', r.returncode == 0, r.stderr)
check('forkpool starts 7 children (6 + 1 retry)', len(re.findall(r'start job', out)) == 7, out)
check('forkpool job 3 fails once then retried', re.search(r'FAIL  job 3 .*exit 1\) -> retry', out) and 'start job 3 (try 2)' in out, out)
check('forkpool all 6 done', len(re.findall(r'done  job', out)) == 6, out)
check('forkpool never runs more than 2 at once', all(
    sum(1 for l in out.splitlines()[:i+1] if 'start' in l) - sum(1 for l in out.splitlines()[:i+1] if ('done' in l or 'FAIL' in l)) <= 2
    for i in range(len(out.splitlines()))), out)
t1, t2, t6 = total_ms(run('forkpool', '6', '1').stdout), total_ms(out), total_ms(run('forkpool', '6', '6').stdout)
check('forkpool MAXPAR 1 ~ 2600 ms', t1 and 2400 <= t1[0] <= 3200, t1)
check('forkpool MAXPAR 6 ~ 600 ms', t6 and 550 <= t6[0] <= 1000, t6)
check('forkpool more slots -> faster', t1 and t2 and t6 and t1[0] > t2[0] > t6[0])
check('forkpool usage -> 2', run('forkpool').returncode == 2)

# sigctl
out = run('sigctl').stdout
check('sigctl term: stopped by SIGSTOP', re.search(r'stopped by signal \d+', out), out)
check('sigctl term: continued', 'continued' in out, out)
rep = [int(x) for x in re.findall(r'report: (\d+) ticks', out)]
check('sigctl term: two reports', len(rep) == 2, out)
check('sigctl term: no progress while stopped (second report < first + 6)', len(rep) == 2 and rep[1] < rep[0] + 6, rep)
check('sigctl term: child cleans up and exits 0', 'got SIGTERM: cleanup' in out and 'exited with 0' in out, out)
out = run('sigctl', 'kill').stdout
check('sigctl kill: killed by signal 9, no cleanup', 'killed by signal 9' in out and 'cleanup' not in out, out)
out = run('sigctl', 'late').stdout
check('sigctl late: child killed by SIGUSR1 default action', re.search(r'killed by signal \d+ \(User defined signal 1\)', out), out)

print(f'\n{PASS} passed, {FAIL} failed')
sys.exit(1 if FAIL else 0)

#!/usr/bin/env python3
"""Process deep dive: demand paging, /proc/PID/stat, CPU vs wait,
zombie/orphan lifetime, and resource-limit violations."""
import os
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time
from pathlib import Path

BASE = Path(__file__).resolve().parent
LINUX = sys.platform.startswith('linux')
runs = 0

def run(name, *args, status=0, timeout=60):
    global runs
    p = subprocess.run([str(BASE / name), *map(str, args)], capture_output=True,
                       text=True, timeout=timeout)
    assert p.returncode == status, (name, args, p.returncode, p.stdout, p.stderr)
    runs += 1
    return p.stdout, p.stderr

def fields(text):
    return dict(re.findall(r'(\w+)=(\S+)', text))

def stages(text):
    return {m[0]: m for m in re.findall(
        r'stage=(\w+)\s+rss_kib=(\S+)\s+minflt=(\d+)\s+majflt=(\d+)', text)}

# pagetouch: malloc reserves, touching pages makes them resident.
out, _ = run('pagetouch', '64')
head = fields(out.splitlines()[0])
pages = int(head['pages'])
st = stages(out)
assert list(st) == ['start', 'after_malloc', 'touch_half', 'touch_all',
                    'touch_again', 'after_free'], out
flt = {k: int(v[2]) for k, v in st.items()}
assert flt['after_malloc'] - flt['start'] < 16, out
assert flt['touch_all'] - flt['after_malloc'] >= pages * 0.9, out
assert flt['touch_again'] - flt['touch_all'] < 16, out
if LINUX:
    rss = {k: int(v[1]) for k, v in st.items()}
    assert rss['after_malloc'] - rss['start'] < 1024, out
    assert rss['touch_all'] - rss['after_malloc'] >= 60 * 1024, out
    assert rss['touch_half'] < rss['touch_all'], out
    assert rss['after_free'] < rss['touch_all'] - 60 * 1024, out
else:
    assert st['start'][1] == 'unavailable'
run('pagetouch', '0', status=2)
run('pagetouch', '1', '2', status=2)

# cpu_wait: busy is user time, sys is kernel time, sleep is neither.
out, _ = run('cpu_wait')
modes = {m[0]: [float(x) for x in m[1:4]] + [int(m[4]), int(m[5])]
         for m in re.findall(r'mode=(\w+)\s+wall=(\S+) user=(\S+) sys=(\S+)'
                             r' nvcsw=(\d+) nivcsw=(\d+)', out)}
assert set(modes) == {'busy', 'sys', 'sleep'}, out
# 각 값: [wall, user, sys, nvcsw, nivcsw]
busy, sysm, sleep = modes['busy'], modes['sys'], modes['sleep']
assert busy[1] > 0.5 and busy[2] < 0.3, out
assert sysm[2] > 0.3, out
assert sleep[1] + sleep[2] < 0.2, out
if LINUX:
    assert sleep[3] >= 50 and busy[3] < 10, out
out, _ = run('cpu_wait', 'sleep')
assert 'mode=sleep' in out and 'mode=busy' not in out
run('cpu_wait', 'nap', status=2)

# pstat: parse /proc/PID/stat, including a comm with ') ' inside.
if LINUX:
    out, _ = run('pstat', 'self')
    a = fields(out)
    assert a['comm'] == '(pstat)' and a['state'] == 'R', out
    assert int(a['clk_tck']) > 0 and int(a['rss_kib']) > 0
    busy_proc = subprocess.Popen(['python3', '-c', 'while True: pass'])
    try:
        time.sleep(0.5)
        out, _ = run('pstat', busy_proc.pid, '2')
        cpus = [int(x) for x in re.findall(r'cpu=\s*(\d+)%', out)]
        assert len(cpus) == 2 and max(cpus) >= 50, out
    finally:
        busy_proc.kill(); busy_proc.wait()
    with tempfile.TemporaryDirectory() as d:
        odd = Path(d) / 'a) b'
        shutil.copy('/bin/sleep', odd)
        p = subprocess.Popen([str(odd), '5'])
        try:
            time.sleep(0.3)
            out, _ = run('pstat', p.pid)
            assert 'comm=(a) b)' in out and 'state=S' in out, out
        finally:
            p.kill(); p.wait()
    _, err = run('pstat', '999999999', status=1)
    assert 'cannot read' in err
else:
    out, _ = run('pstat', 'self')
    assert 'stat=unsupported' in out
run('pstat', status=2)
run('pstat', 'abc', status=2)
run('pstat', 'self', '0', status=2)

# reap_show: the child is a zombie until the parent reaps it.
p = subprocess.Popen([str(BASE / 'reap_show'), 'zombie', '3'],
                     stdout=subprocess.PIPE, text=True)
line = p.stdout.readline()
child = int(fields(line)['child'])
time.sleep(0.5)
stat = subprocess.run(['ps', '-o', 'stat=', '-p', str(child)],
                      capture_output=True, text=True).stdout.strip()
assert stat.startswith('Z'), (line, stat)
rest = p.stdout.read()
assert p.wait(timeout=10) == 0
assert f'reaped child={child} exit_status=7' in rest, rest
gone = subprocess.run(['ps', '-o', 'stat=', '-p', str(child)],
                      capture_output=True, text=True).stdout.strip()
assert gone == '', gone
runs += 1
with tempfile.TemporaryFile('w+') as f:
    rc = subprocess.run([str(BASE / 'reap_show'), 'orphan'], stdout=f).returncode
    assert rc == 0
    time.sleep(3)
    f.seek(0)
    out = f.read()
before = int(re.search(r'ppid_before=(\d+)', out).group(1))
after = int(re.search(r'ppid_after=(\d+)', out).group(1))
assert before != after and f'parent={before}' in out, out
runs += 1
run('reap_show', status=2)
run('reap_show', 'zombie', 'x', status=2)

# limit_lab: crossing each limit has a distinct, observable outcome.
out, _ = run('limit_lab', 'nofile', '16')
assert 'open failed after 13 opens' in out and 'errno=%d' % 24 in out, out
assert 'last_fd=15 soft_limit=16' in out and 'closed=13' in out
if LINUX:
    out, _ = run('limit_lab', 'as', '64')
    assert 'malloc(128 MiB) = NULL errno=12' in out, out
p = subprocess.run([str(BASE / 'limit_lab'), 'cpu', '1'], capture_output=True,
                   text=True, timeout=30)
assert p.returncode == -signal.SIGXCPU, (p.returncode, p.stdout, p.stderr)
assert 'spinning past 1 s' in p.stdout
runs += 1
run('limit_lab', 'nofile', '3', status=2)
run('limit_lab', 'disk', '5', status=2)

print(f'PASS: {runs} runs; demand paging, stat parsing, user/sys/wait, '
      'zombie/orphan, EMFILE/ENOMEM/SIGXCPU')

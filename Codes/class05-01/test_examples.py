#!/usr/bin/env python3
"""System identification, sysconf contracts, resource limits, /proc observation."""
import platform
import re
import resource
import subprocess
from pathlib import Path

BASE = Path(__file__).resolve().parent
runs = 0

def run(name, *args, status=0):
    global runs
    p = subprocess.run([str(BASE / name), *map(str, args)], capture_output=True,
                       text=True, timeout=30)
    assert p.returncode == status, (name, args, p.returncode, p.stdout, p.stderr)
    runs += 1
    return p.stdout, p.stderr

def fields(text):
    return dict(re.findall(r'(\w+)=(\S+)', text))

LINUX = platform.system() == 'Linux'

out, _ = run('sysprobe')
a = fields(out)
assert a['sysname'] == platform.system()
assert a['machine'] == platform.machine()
assert int(a['pagesize']) > 0 and int(a['pagesize']) % 2 == 0
assert int(a['clk_tck']) > 0
assert int(a['cpus_online']) >= 1
assert int(a['open_max']) >= 3
soft, hard = resource.getrlimit(resource.RLIMIT_NOFILE)
expected_open = 'unlimited' if soft == resource.RLIM_INFINITY else str(int(soft))
assert a['open_max'] == expected_open
run('sysprobe', 'extra', status=2)

out, _ = run('limit_show')
lines = {f.split()[0]: f.split()[1:] for f in out.splitlines()}
soft, hard = resource.getrlimit(resource.RLIMIT_NOFILE)
want = ('soft=unlimited' if soft == resource.RLIM_INFINITY
        else f'soft={soft}')
assert lines['nofile'][0] == want
soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
want = ('soft=unlimited' if soft == resource.RLIM_INFINITY
        else f'soft={soft}')
assert lines['stack'][0] == want
for name in ('as', 'fsize', 'cpu', 'nproc', 'core'):
    assert len(lines[name]) == 2, name

out, _ = run('nofile_demo')
soft, hard = resource.getrlimit(resource.RLIMIT_NOFILE)
if soft != resource.RLIM_INFINITY:
    before_soft = int(re.search(r'before soft=(\d+)', out).group(1))
    opened = int(re.search(r'opened=(\d+)', out).group(1))
    assert opened <= before_soft
    assert re.search(r'stopped=(EMFILE|cap)', out)
assert 'closed all descriptors' in out

out, _ = run('nofile_demo', '--raise', '64')
r = re.search(r'after\s+soft=(\d+)', out)
assert r and int(r.group(1)) == 64
opened = int(re.search(r'opened=(\d+)', out).group(1))
assert opened <= 64
run('nofile_demo', '--raise', 'not-a-number', status=2)
run('nofile_demo', 'bogus', status=2)

out, err = run('proc_probe', status=0 if LINUX else 1)
if LINUX:
    assert re.search(r'VmRSS:\s+\d+ kB', out)
    assert re.search(r'Threads:\s+\d+', out)
    assert re.match(r'\d+\.\d+ \d+\.\d+ \d+\.\d+ ', out.split('loadavg]')[1].strip())
    assert re.search(r'count=\d+', out)
    assert int(re.search(r'count=(\d+)', out).group(1)) >= 3
else:
    assert '/proc/self/status' in err

out, err = run('sysinfo_demo', status=0 if LINUX else 1)
if LINUX:
    s = fields(out)
    assert int(s['uptime']) >= 0 and int(s['procs']) >= 1
    assert int(s['mem_unit']) >= 1
    assert int(s['totalram']) > int(s['freeram']) >= 0
    assert re.search(r'loadavg=\d+\.\d\d \d+\.\d\d \d+\.\d\d', out)
else:
    assert 'Linux-only' in out

print(f'PASS: {runs} process runs; uname, sysconf, rlimits, fd ceiling, /proc observation'
      + (' (Linux)' if LINUX else ' (macOS: /proc and sysinfo branches verified)'))

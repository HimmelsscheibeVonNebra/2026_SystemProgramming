#!/usr/bin/env python3
"""Process state, address space, and resource observation."""
import os
import re
import subprocess
import tempfile
import time
from pathlib import Path

BASE = Path(__file__).resolve().parent
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

def ps_stat(pid, want):
    """STAT가 want로 시작할 때까지 최대 6회 관찰한다(경합 여유)."""
    for _ in range(6):
        p = subprocess.run(['ps', '-o', 'stat=', '-p', str(pid)],
                           capture_output=True, text=True, timeout=10)
        if p.stdout.strip().startswith(want):
            return True
        time.sleep(0.4)
    return False

# addr_show: storage-class regions appear as distinct addresses.
out, _ = run('addr_show')
a = {k: v for k, v in re.findall(r'(\w+)=(0x[0-9a-fA-F]+)', out)}
for key in ('data', 'bss', 'stack', 'pointer_on_stack', 'heap1', 'heap2'):
    assert key in a, (key, out)
assert len({a['data'], a['bss'], a['stack'], a['heap1']}) == 4
assert 'observe: cat /proc/' in out
assert int(fields(out)['pid']) > 0
run('addr_show', 'extra', status=2)

# maps_show: /proc/self/maps walk, or graceful unsupported notice.
if Path('/proc/self/maps').exists():
    out, _ = run('maps_show')
    m = fields(out)
    assert int(m['regions']) >= 1
    assert int(m['stack_regions']) >= 1, out
    assert '[stack]' in out
else:
    out, _ = run('maps_show')
    assert 'maps=unsupported' in out and 'no /proc' in out
run('maps_show', 'x', status=2)

# state_show: sleep mode shows S, busy mode shows R in ps STAT.
out, _ = run('state_show', '2')
a = fields(out)
assert int(a['pid']) > 0 and a['mode'] == 'sleep'
assert 'observe: ps -o pid,ppid,stat' in out
assert 'tick=2 of 2' in out
out, _ = run('state_show', '--busy', '2')
assert 'mode=busy' in out and 'tick=2 of 2' in out
proc = subprocess.Popen([str(BASE / 'state_show'), '10'],
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
time.sleep(1.2)
assert ps_stat(proc.pid, 'S') is True, proc.pid
proc.kill(); proc.wait()
proc = subprocess.Popen([str(BASE / 'state_show'), '--busy', '10'],
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
time.sleep(1.2)
assert ps_stat(proc.pid, 'R') is True, proc.pid
proc.kill(); proc.wait()
runs += 2
run('state_show', '0', status=2)
run('state_show', '--busy', '3', 'x', status=2)

# hold_open: open returns the smallest free descriptor, then closes.
tmp_fd, tmp_name = tempfile.mkstemp(prefix='hold_', dir=BASE)
os.close(tmp_fd)
tmp = Path(tmp_name)
try:
    out, _ = run('hold_open', tmp, '1')
    a = fields(out)
    assert int(a['fd']) >= 3
    assert 'observe: ls -l /proc' in out
    assert 'hold=1 of 1' in out and f"closed fd={a['fd']}" in out
    _, err = run('hold_open', '/no/such/file', '1', status=1)
    assert 'No such file or directory' in err
    run('hold_open', status=2)
    run('hold_open', tmp, '0', status=2)
finally:
    tmp.unlink()

# procdump: standard-API resource facts plus optional /proc detail.
out, _ = run('procdump')
a = fields(out)
assert int(a['pid']) > 0 and int(a['ppid']) >= 1
assert int(a['uid']) == os.getuid() and int(a['euid']) == os.geteuid()
assert re.match(r'^\d+$|unlimited$', a['rlimit_nofile_soft'])
assert re.match(r'^\d+$|unlimited$', a['rlimit_as_soft'])
assert re.match(r'^\d+\.\d{6}$', a['cpu_user'])
assert int(a['maxrss']) >= 0
assert a['unit'] in ('bytes', 'KiB')
if Path('/proc/self/status').exists():
    assert a['status_State'] and 'status_VmRSS=' in out
    assert int(a['fd_count']) >= 4
else:
    assert a['proc_status'] == 'unavailable'
    assert a['fd_count'] == 'unavailable'
run('procdump', 'x', status=2)

print(f'PASS: {runs} runs; storage regions, maps walk, S/R states in ps, '
      'fd lifetime, rlimit/rusage report')

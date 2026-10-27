#!/usr/bin/env python3
"""Blocked/pending signals, sigsuspend, safe handlers, SIGCHLD, USR rounds."""
import os
import select
import signal
import subprocess
import time
from pathlib import Path

BASE = Path(__file__).resolve().parent
LINUX = Path('/proc/self/status').exists()
runs = 0

def run(name, *args, status=0, timeout=60):
    global runs
    p = subprocess.run([str(BASE / name), *map(str, args)], capture_output=True,
                       text=True, timeout=timeout)
    assert p.returncode == status, (name, args, p.returncode, p.stdout, p.stderr)
    runs += 1
    return p.stdout, p.stderr

def popen(name, *args):
    return subprocess.Popen([str(BASE / name), *map(str, args)],
                            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)

class Lines:
    """파이프에서 줄 단위로 읽는다(select와 자체 버퍼, 제한 시간 있음)."""
    def __init__(self, fd):
        self.fd = fd
        self.buf = b''
        self.eof = False

    def readline(self, timeout=10):
        while b'\n' not in self.buf and not self.eof:
            if not select.select([self.fd], [], [], timeout)[0]:
                raise AssertionError('timeout waiting for line')
            chunk = os.read(self.fd, 4096)
            if not chunk:
                self.eof = True
                break
            self.buf += chunk
        if not self.buf:
            raise AssertionError('unexpected EOF')
        i = self.buf.find(b'\n')
        if i == -1:
            line, self.buf = self.buf, b''
        else:
            line, self.buf = self.buf[:i + 1], self.buf[i + 1:]
        return line.decode()

    def rest(self, timeout=15):
        """남은 출력을 끝까지 읽는다(EOF 또는 제한 시간까지)."""
        while not self.eof:
            if not select.select([self.fd], [], [], timeout)[0]:
                break
            chunk = os.read(self.fd, 4096)
            if not chunk:
                self.eof = True
                break
            self.buf += chunk
        tail, self.buf = self.buf, b''
        return tail.decode()

# block_pending: Ctrl-C during the critical section stays pending; the
# handler runs only when the mask is restored.
p = popen('block_pending', 'hold', '3')
out_lines = Lines(p.stdout.fileno())
assert 'SIGINT blocked for 3s' in out_lines.readline(), p.pid
assert 'observe: ./block_pending show' in out_lines.readline(), p.pid
assert 'sigmask_show' in out_lines.readline(), p.pid
os.kill(p.pid, signal.SIGINT)
line = out_lines.readline(timeout=5)
assert 'pending' in line and 'NOT running' in line, p.pid
p.wait(timeout=15)
rest = out_lines.rest()
assert 'hold over: unblocking' in rest, rest
assert '[handler] SIGINT delivered' in rest, rest
assert 'after unblock: ints=1' in rest, rest
assert p.returncode == 0
runs += 1

out, _ = run('block_pending', 'hold', '1')
assert 'after unblock: ints=0' in out, out

if LINUX:
    out, _ = run('block_pending', 'show', 'self')
    assert 'SigPnd' in out and 'SigBlk' in out, out
else:
    out, _ = run('block_pending', 'show', '1')
    assert 'show=unsupported' in out and 'no /proc' in out, out
run('block_pending', status=2)
run('block_pending', 'hold', '99', status=2)
run('block_pending', 'hold', 'x', status=2)
run('block_pending', 'show', status=2)

# suspend_demo: a pending blocked signal cannot wake pause but wakes
# sigsuspend at once, and the mask comes back.
out, _ = run('suspend_demo', 'suspend')
assert 'pending (blocked)' in out and 'usr1=1' in out, out
assert 'woke at once' in out and 'after 0.0' in out, out
assert 'mask restored: SIGUSR1 blocked again' in out, out
out, _ = run('suspend_demo', 'pause')
assert 'cannot wake it' in out and 'woke by ALRM' in out, out
assert 'after 3.' in out, out
assert 'usr1=1' in out and 'after restore: usr1=1' in out, out
run('suspend_demo', status=2)
run('suspend_demo', 'hang', status=2)

# safe_flag: the handler only writes and bumps the flag; errno survives.
out, _ = run('safe_flag', 'self', '5')
assert out.count('saw flag=') == 5, out
assert 'total ints=5' in out, out
assert 'errno preserved across handler: yes' in out, out
out, _ = run('safe_flag', 'wait', '2')
assert 'received 0 SIGINT in 2s' in out, out
p = popen('safe_flag', 'wait', '4')
out_lines = Lines(p.stdout.fileno())
assert 'waiting 4s' in out_lines.readline(), p.pid
time.sleep(0.6)
os.kill(p.pid, signal.SIGINT)
p.wait(timeout=15)
rest = out_lines.rest()
assert 'received=1' in rest and 'received 1 SIGINT in 4s' in rest, rest
assert p.returncode == 0
runs += 1
run('safe_flag', status=2)
run('safe_flag', 'self', '100', status=2)
run('safe_flag', 'bogus', '2', status=2)

# chld_reap: zombies appear and are reaped via waitpid(-1, WNOHANG).
out, _ = run('chld_reap', '4')
assert out.count('reaped child=') == 4, out
for raw in ('0x0100', '0x0200', '0x0300', '0x0400'):
    assert f'raw status={raw}' in out, out
assert 'ECHILD' in out and 'no zombies' in out, out
run('chld_reap', '0', status=2)
run('chld_reap', '9', status=2)

# usr_round: kill() targets a PID; parent and child count their rounds.
out, _ = run('usr_round', '2')
assert 'round 1/2' in out and 'round 2/2' in out, out
assert out.count('[child handler]') == 2, out
assert out.count('[parent handler]') == 2, out
assert 'done: received 2 USR1, replied 2 USR2' in out, out
assert 'exited, status=0' in out, out
out, _ = run('usr_round')
assert 'round 4/4' in out and 'exited, status=0' in out, out
run('usr_round', '0', status=2)
run('usr_round', '11', status=2)

note = '' if LINUX else ' (no /proc on this host; show mode reports unsupported)'
print(f'PASS: {runs} runs; block/pending, pause vs sigsuspend, '
      f'safe handler flags, SIGCHLD reaping, USR1/2 rounds{note}')

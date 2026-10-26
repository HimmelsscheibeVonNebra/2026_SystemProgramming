#!/usr/bin/env python3
"""Exit status, signal handling, and /proc signal masks."""
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

def pipe_shell(cmd, status=0, timeout=60):
    """셸 파이프라인으로 실제 파이프 입력을 만들어 실행한다."""
    global runs
    p = subprocess.run(['sh', '-c', cmd], capture_output=True, text=True,
                       timeout=timeout, cwd=BASE)
    assert p.returncode == status, (cmd, p.returncode, p.stdout, p.stderr)
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

# exit_how: fork/waitpid reports how the child ended as a raw status word.
out, _ = run('exit_how', 'exit')
assert 'exited, status=3' in out and 'raw status=0x0300' in out, out
out, _ = run('exit_how', 'abort')
assert 'killed by signal 6' in out, out
if LINUX:
    assert 'raw status=0x0086' in out and 'core dumped' in out, out
else:
    assert 'raw status=0x0006' in out, out
out, _ = run('exit_how', 'segv')
assert 'killed by signal 11' in out, out
if LINUX:
    assert 'raw status=0x008b' in out and 'core dumped' in out, out
else:
    assert 'raw status=0x000b' in out, out
out, _ = run('exit_how', 'term')
assert 'killed by signal 15' in out and 'raw status=0x000f' in out, out
run('exit_how', 'bogus', status=2)
run('exit_how', status=2)

# exit_how wait: STOP and CONT are reported too, like a job-control shell.
p = popen('exit_how', 'wait')
out_lines = Lines(p.stdout.fileno())
line = out_lines.readline()
assert 'child=' in line and 'waiting' in line, line
child = int(line.split('child=')[1].split()[0])
os.kill(child, signal.SIGSTOP)
assert 'stopped by signal' in out_lines.readline(), p.pid
assert 'raw status=0x' in out_lines.readline(), p.pid
os.kill(child, signal.SIGCONT)
if LINUX:                       # macOS waitpid는 WCONTINUED를 보고하지 않는다
    assert 'continued' in out_lines.readline(), p.pid
    assert 'raw status=0x' in out_lines.readline(), p.pid
os.kill(child, signal.SIGKILL)
assert 'killed by signal 9' in out_lines.readline(), p.pid
assert 'raw status=0x0009' in out_lines.readline(), p.pid
p.wait(timeout=10)
assert p.returncode == 0
runs += 1

# catch_int: handler counts SIGINT, ignores SIGQUIT, SIGKILL is rejected.
p = popen('catch_int')
out_lines = Lines(p.stdout.fileno())
assert 'sigaction(SIGKILL): Invalid argument' in out_lines.readline(), p.pid
assert 'pid=' in out_lines.readline(), p.pid
for _ in range(3):
    time.sleep(0.2)
    os.kill(p.pid, signal.SIGINT)
p.wait(timeout=10)
rest = out_lines.rest()
assert '[handler] SIGINT' in rest, rest
for k in (1, 2, 3):
    assert f'main: SIGINT #{k}' in rest, rest
assert 'SIGINT x3 -> exit 0' in rest and p.returncode == 0, rest
runs += 1

p = popen('catch_int')
out_lines = Lines(p.stdout.fileno())
out_lines.readline()
out_lines.readline()
os.kill(p.pid, signal.SIGQUIT)          # SIG_IGN으로 등록되어 무시된다
time.sleep(0.5)
assert p.poll() is None, 'SIGQUIT was not ignored'
os.kill(p.pid, signal.SIGTERM)
p.wait(timeout=10)
rest = out_lines.rest()
assert 'SIGTERM -> cleanup, exit 0' in rest and p.returncode == 0, rest
runs += 1

# timeout_read: alarm interrupts read; SA_RESTART resumes it.
pipe_shell('(sleep 4; echo hello) | ./timeout_read default 2', status=142)
out, _ = pipe_shell('(sleep 4; echo hello) | ./timeout_read norestart 2',
                    status=1)
assert '[handler] SIGALRM' in out and 'read: -1' in out and 'EINTR' in out, out
assert 'after 2.' in out, out
out, _ = pipe_shell('(sleep 4; echo hello) | ./timeout_read restart 2')
assert 'read: 6 bytes after 4.' in out and 'alarms=1' in out, out
run('timeout_read', status=2)
run('timeout_read', 'default', '0', status=2)
run('timeout_read', 'bogus', '2', status=2)

# sigmask_show: /proc signal mask lines, or a graceful failure without it.
if LINUX:
    out, _ = run('sigmask_show')
    for key in ('SigPnd', 'SigBlk', 'SigIgn', 'SigCgt'):
        assert key in out, out
    out, _ = run('sigmask_show', os.getpid())
    assert 'Name:' in out and 'Pid:' in out, out
else:
    out, err = run('sigmask_show', status=1)
    assert '/proc' in err, err
run('sigmask_show', '1', '2', status=2)

# sig_names: number-name-action reference table.
out, _ = run('sig_names')
for row in ('1) HUP', '2) INT', '9) KILL', '11) SEGV', '15) TERM', '31) SYS'):
    assert row in out, out
assert 'RT' in out, out
for num, act in (('9', 'Term'), ('11', 'Core'), ('17', 'Ignore'),
                 ('18', 'Cont'), ('19', 'Stop'), ('15', 'Term')):
    out, _ = run('sig_names', num)
    assert f'default={act}' in out, out
run('sig_names', '0', status=2)
run('sig_names', '32', status=2)
run('sig_names', 'x', status=2)
run('sig_names', '1', '2', status=2)

note = '' if LINUX else ' (no /proc on this host; sigmask_show fails as documented)'
print(f'PASS: {runs} runs; raw exit status, sigaction/EINVAL, EINTR vs '
      f'SA_RESTART, /proc masks, signal table{note}')

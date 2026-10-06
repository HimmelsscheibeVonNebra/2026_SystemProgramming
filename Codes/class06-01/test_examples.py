#!/usr/bin/env python3
"""Process identity, arguments, environment, and working directory."""
import os
import re
import subprocess
from pathlib import Path

BASE = Path(__file__).resolve().parent
runs = 0

def run(name, *args, status=0, env=None, cwd=None, stdin=None):
    global runs
    p = subprocess.run([str(BASE / name), *map(str, args)], capture_output=True,
                       text=True, timeout=30, env=env, cwd=cwd, stdin=stdin)
    assert p.returncode == status, (name, args, p.returncode, p.stdout, p.stderr)
    runs += 1
    return p.stdout, p.stderr

def fields(text):
    return dict(re.findall(r'(\w+)=(\S+)', text))

# id_probe: identity queries match the surrounding process state.
out, _ = run('id_probe')
a = fields(out)
assert int(a['pid']) > 0
assert int(a['ppid']) >= 1
assert int(a['pgid']) == os.getpgrp()
assert int(a['sid']) == os.getsid(0)
assert 'tty=' in out and 'compare: ps -o' in out
out, _ = run('id_probe', stdin=subprocess.DEVNULL)
assert 'tty=(none)' in out          # 표준 입력이 터미널이 아니면 NULL
run('id_probe', 'extra', status=2)

# arg_show: argv is a NULL-terminated pointer array; argv[0] is not guaranteed.
out, _ = run('arg_show', 'one', 'two', 'three')
assert 'argc=4' in out
assert out.count('argv[0]=') == 1 and 'arg_show' in out.splitlines()[1]
assert 'argv[3]=three' in out
assert re.search(r'argv\[4\]=\(nil\)|argv\[4\]=0x0', out)
p = subprocess.run(['bash', '-c',
                    f"exec -a fake-name {BASE / 'arg_show'} x"],
                   capture_output=True, text=True, timeout=30)
assert p.returncode == 0 and 'argv[0]=fake-name' in p.stdout
runs += 1

# env_dump: environ walk plus getenv/setenv/unsetenv contracts.
out, _ = run('env_dump')
count = int(re.search(r'env_count=(\d+)', out).group(1))
assert count > 0 and len(out.splitlines()) == count + 1
out, _ = run('env_dump', '--find', 'PROBE_MESSAGE', env={'PATH': '/usr/bin'})
assert 'PROBE_MESSAGE=(unset)' in out
out, _ = run('env_dump', '--find', 'PATH', env={'PATH': '/usr/bin'})
assert 'PATH=/usr/bin' in out
out, _ = run('env_dump', '--set', 'PROBE_MESSAGE=hi', env={'PATH': '/usr/bin'})
assert 'PROBE_MESSAGE=hi' in out and 'env_count=2' in out
out, _ = run('env_dump', '--unset', 'PATH', env={'PATH': '/usr/bin', 'X': '1'})
assert 'PATH=(unset)' in out and 'env_count=1' in out
run('env_dump', '--set', 'NOEQ', status=2)
run('env_dump', '--bogus', 'x', status=2)

# cwd_probe: getcwd in two forms and chdir success/failure paths.
out, _ = run('cwd_probe')
here = os.path.realpath(os.getcwd())
assert f'cwd={here}' in out and f'cwd_alloc={here}' in out
out, _ = run('cwd_probe', '/tmp')
assert 'after_chdir=/private/tmp' in out or 'after_chdir=/tmp' in out
_, err = run('cwd_probe', '/no/such/dir', status=1)
assert 'No such file or directory' in err
_, err = run('cwd_probe', '/etc/hosts', status=1)
assert 'Not a directory' in err
run('cwd_probe', 'a', 'b', status=2)

# procinfo: full execution context in one report.
out, _ = run('procinfo', 'one')
a = fields(out)
assert int(a['pid']) > 0 and int(a['ppid']) >= 1
assert int(a['pgid']) == os.getpgrp() and int(a['sid']) == os.getsid(0)
assert int(a['uid']) == os.getuid() and int(a['euid']) == os.geteuid()
assert int(a['gid']) == os.getgid() and int(a['egid']) == os.getegid()
m = re.search(r'^cwd=(.*)$', out, re.M)
assert m and m.group(1) == os.path.realpath(os.getcwd())
assert 'argc=2' in out and int(a['env_count']) > 0
assert 'HOME=' in out and 'PATH=' in out
assert 'argv[1]=one' not in out
out, _ = run('procinfo', 'one', '--argv')
assert 'argv[1]=one' in out and 'argv[2]=--argv' in out
out, _ = run('procinfo', '--env', env={'PATH': '/usr/bin', 'K': 'v'})
assert out.count('\n') >= 4 and 'K=v' in out and 'PATH=/usr/bin' in out
out, _ = run('procinfo', '--env', env={})
assert 'HOME=(unset)' in out and 'PATH=(unset)' in out and 'env_count=0' in out
run('procinfo', '--bogus', status=2)

print(f'PASS: {runs} process runs; pid/ppid/pgid/sid, argv[0] override, '
      'environ contracts, getcwd/chdir paths, context summary')

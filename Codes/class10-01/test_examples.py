#!/usr/bin/env python3
"""fork · exec · wait 예제 7개의 동작을 확인한다.

예제 출처: https://github.com/bluekds/SystemProgramming/tree/main/Lecture7
이 스크립트는 예제를 파이프에 연결해 실행하므로, exec 직전의 printf 출력이
stdio 버퍼에 남아 있다가 exec와 함께 사라지는 동작(완전 버퍼링)도 함께 확인한다.
터미널에서 직접 실행하면(줄 버퍼링) 그 줄은 보인다 — 교안 2부의 관찰 포인트다.
"""
import os
import re
import subprocess
import time
from pathlib import Path

BASE = Path(__file__).resolve().parent
runs = 0


def run(name, *args, status=0, timeout=60, env=None):
    global runs
    p = subprocess.run([str(BASE / name), *map(str, args)], capture_output=True,
                       text=True, timeout=timeout, cwd=BASE, env=env)
    assert p.returncode == status, (name, args, p.returncode, p.stdout, p.stderr)
    runs += 1
    return p.stdout, p.stderr


# fork: 한 번 호출, 두 번 반환. 두 프로세스가 각각 "End of fork"를 찍는다.
out, _ = run('fork')
parent = re.search(r"Parent process - My PID:(\d+), My Parent's PID:(\d+), My Child's PID:(\d+)", out)
child = re.search(r"Child Process - My PID:(\d+), My Parent's PID:(\d+)", out)
assert parent and child, out
assert int(parent.group(2)) == os.getpid(), (parent.group(2), os.getpid())   # 부모의 부모 = 이 스크립트
assert parent.group(3) == child.group(1), out                                 # 부모가 받은 반환값 = 자식의 PID
assert child.group(2) == parent.group(1), out                                 # 자식의 PPID = 부모의 PID
assert out.count('End of fork') == 2, out

# execlp: PATH에서 ls를 찾아 교체한다. 성공하면 "After"는 영원히 찍히지 않는다.
out, err = run('execlp')
assert 'After exec function' not in out and err == '', (out, err)
assert '.\n' in out and 'execlp.c' in out, out                                # ls -a의 결과가 우리 출력 대신 나온다
assert 'Before exec function' not in out, out                                 # 파이프(완전 버퍼)라 exec가 버퍼를 버렸다

# execlp 실패: 찾지 못하면 그때만 -1로 돌아오고 perror 후 exit(1). 버퍼의 Before는 exit가 비운다.
env = dict(os.environ, PATH='/nonexistent')
out, err = run('execlp', status=1, env=env)
assert 'Before exec function' in out and 'After exec function' not in out, out
assert err.startswith('execlp: '), err

# execv: 경로를 직접 주고 인자 배열을 넘긴다. 결과는 execlp와 같다.
out, err = run('execv')
assert 'After exec function' not in out and err == '', (out, err)
assert 'execv.c' in out and 'Before exec function' not in out, out

# forkNexec: 부모는 PID만 찍고 끝나고, 자식은 ls -a로 바뀐다. 파이프라 자식의 "--> Child Process"는 사라진다.
out, err = run('forkNexec')
assert re.search(r'--> Parent process - My PID:\d+', out), out
assert 'forkNexec.c' in out and err == '', (out, err)
assert '--> Child Process' not in out, out

# wait: 자식 exit(2) → 부모 wait → status 512 = 0x0200 (상위 바이트가 종료 코드).
out, _ = run('wait')
assert out.splitlines() == ['--> Child Process', '--> Parent process', 'Status: 512, 200',
                            'Child process Exit Status:2'], out

# waitpid(WNOHANG): 자식이 3초 자는 동안 1초마다 찔러 보고(0 반환), 끝나면 exit 3을 받는다.
t0 = time.monotonic()
out, _ = run('waitpid')
elapsed = time.monotonic() - t0
lines = out.splitlines()
assert lines[0] == '--> Parent process' or lines[0] == '--> Child process', out
assert 3 <= out.count('Parent still wait...') <= 5, out
assert lines[-1] == 'Child Exit Status : 3', out
assert 2.5 < elapsed < 10, elapsed

# atexit: 등록의 역순으로 호출된다.
out, _ = run('atexit')
assert out == 'Cleanup 2 is called.\nCleanup 1 is called.\n', out

print(f'ok ({runs} runs)')

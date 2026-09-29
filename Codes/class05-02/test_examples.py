#!/usr/bin/env python3
"""Calendar time, elapsed time measurement, timestamped logging, clock survey."""
import os
import platform
import re
import subprocess
import tempfile
import time as pytime
from pathlib import Path

BASE = Path(__file__).resolve().parent
runs = 0

def run(name, *args, status=0, tz=None):
    global runs
    env = dict(os.environ)
    if tz is not None:
        env['TZ'] = tz
    p = subprocess.run([str(BASE / name), *map(str, args)], capture_output=True,
                       text=True, timeout=60, env=env)
    assert p.returncode == status, (name, args, p.returncode, p.stdout, p.stderr)
    runs += 1
    return p.stdout, p.stderr

STAMP = r'\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}'

# calendar_demo: epoch는 실제 시각과 일치하고 TZ가 local 문자열을 바꾼다
out, _ = run('calendar_demo', tz='Asia/Seoul')
epoch = int(re.search(r'epoch=(-?\d+)', out).group(1))
assert abs(epoch - pytime.time()) < 10
local = re.search(r'local=(' + STAMP + r') (\S+)', out).group(1)
utc = re.search(r'utc=(' + STAMP + ')', out).group(1)
lh, uh = int(local[11:13]), int(utc[11:13])
assert (lh - uh) % 24 == 9, (local, utc)
d = float(re.search(r'difftime=(\d+\.\d)s', out).group(1))
assert 0.0 <= d < 10.0
out, _ = run('calendar_demo', tz='UTC')
assert re.search(r'local=(' + STAMP + ')', out).group(1) == \
       re.search(r'utc=(' + STAMP + ')', out).group(1)
run('calendar_demo', 'extra', status=2)

# tm_normalize: 범위 밖 값이 mktime 정규화로 정리되는지 확인
out, _ = run('tm_normalize', tz='UTC')
cases = [('1월 32일', '2026-02-01'), ('12월 32일', '2027-01-01'),
         ('2025-02-29', '2025-03-01'), ('2024-02-29', '2024-02-29')]
for label, want in cases:
    block = re.search(re.escape(label) + r'.*?mktime 결과 (\d{4}-\d{2}-\d{2})',
                      out, re.S)
    assert block and block.group(1) == want, (label, want, out)
assert 'tm_mon=0' in out and 'tm_mon=1' in out
run('tm_normalize', 'x', status=2)

# elapsed_demo: 측정값 파싱과 min <= median <= max
out, _ = run('elapsed_demo')
vals = [int(v) for v in re.findall(r'run\d+=(\d+)ns', out)]
assert len(vals) == 5 and all(v > 0 for v in vals)
mn = int(re.search(r'min=(\d+)ns', out).group(1))
md = int(re.search(r'median=(\d+)ns', out).group(1))
mx = int(re.search(r'max=(\d+)ns', out).group(1))
assert mn <= md <= mx and mn <= min(vals) and mx >= max(vals)
out, _ = run('elapsed_demo', '--clocks')
assert re.search(r'CLOCK_REALTIME\s+now=\d+\.\d{9} res=\d+\.\d{9}', out)
assert re.search(r'CLOCK_MONOTONIC\s+now=\d+\.\d{9} res=\d+\.\d{9}', out)
run('elapsed_demo', '--nope', status=2)

# tslog: 형식, 경과 시간 단조 증가, 추가 모드
line_re = re.compile(
    r'(' + STAMP + r') (\S+) \+(\d+\.\d{4})s (.+)')
with tempfile.TemporaryDirectory() as tmpdir:
    log = Path(tmpdir) / 'run.log'
    out, err = run('tslog', '--demo', '-o', log, tz='Asia/Seoul')
    lines1 = log.read_text().splitlines()
    assert len(lines1) == 5
    stamps = [line_re.match(l) for l in lines1]
    assert all(stamps) and stamps[0].group(4) == 'start' and stamps[-1].group(4) == 'done'
    elapsed = [float(s.group(3)) for s in stamps]
    assert all(b >= a for a, b in zip(elapsed, elapsed[1:]))
    assert elapsed[-1] > 0.03   # 줄 사이 약 12ms 간격 4번
    run('tslog', '-o', log, 'second run', tz='Asia/Seoul')
    lines2 = log.read_text().splitlines()
    assert len(lines2) == 6 and line_re.match(lines2[-1]).group(4) == 'second run'
out, err = run('tslog', 'hello', 'world', tz='UTC')
assert len(err.splitlines()) == 2
assert line_re.match(err.splitlines()[0]).group(4) == 'hello'
run('tslog', status=2)
run('tslog', '-o', status=2)
run('tslog', '--demo', 'extra', status=2)

# clock_show: 두 표준 시계의 값과 해상도
out, _ = run('clock_show')
for clock in ('CLOCK_REALTIME', 'CLOCK_MONOTONIC'):
    row = re.search(clock + r'\s+now=(-?\d+)\.(\d{9}) res=(\d+)\.(\d+)', out)
    assert row, (clock, out)
    assert int(row.group(1)) > 0 and int(row.group(3)) >= 0
run('clock_show', 'extra', status=2)

print(f'PASS: {runs} process runs; calendar time, tm normalization, '
      'monotonic measurement, timestamped log, clock survey on '
      + platform.system())

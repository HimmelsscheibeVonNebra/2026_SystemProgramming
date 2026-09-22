#!/usr/bin/env python3
"""Observable file attributes; all mutations use disposable test files."""
import os
import re
import stat
import subprocess
import tempfile
from pathlib import Path

BASE = Path(__file__).resolve().parent
runs = 0

def run(name, *args, status=0):
    global runs
    p = subprocess.run([str(BASE / name), *map(str,args)], capture_output=True,
                       text=True, timeout=10)
    assert p.returncode == status, (name,args,p.returncode,p.stdout,p.stderr)
    runs += 1
    return p.stdout

def attrs(path, follow=False):
    out = run('file_attr', *(['--follow'] if follow else []), path)
    return dict(re.findall(r'(\w+)=(\S+)',out))

with tempfile.TemporaryDirectory(prefix='class04-02-attrs-') as d:
    root = Path(d)
    f = root/'space name.txt';f.write_bytes(b'ABC\n');f.chmod(0o640)
    a=attrs(f)
    assert a['type']=='regular' and a['size']=='4' and a['mode']=='0640'
    assert a['rwx']=='rw-r-----'
    assert int(a['uid'])==os.getuid() and int(a['gid'])==f.stat().st_gid
    h=root/'hard';os.link(f,h)
    a,b=attrs(f),attrs(h)
    assert a['inode']==b['inode'] and a['device']==b['device'] and a['links']=='2'
    soft=root/'soft';soft.symlink_to(f.name)
    s=attrs(soft);t=attrs(soft,True)
    assert s['type']=='symlink' and int(s['size'])==len(os.fsencode(f.name))
    assert t['inode']==a['inode'] and t['type']=='regular'
    broken=root/'broken';broken.symlink_to('missing')
    assert attrs(broken)['type']=='symlink'
    run('file_attr','--follow',broken,status=1)
    run('file_attr',root/'missing',status=1)
    run('file_attr',f/'child',status=1)
    assert attrs(root)['type']=='directory'
    fifo=root/'fifo';os.mkfifo(fifo)
    assert attrs(fifo)['type']=='fifo'
    special=root/'special';special.mkdir();special.chmod(0o1777)
    assert attrs(special)['rwx']=='rwxrwxrwt'
    special.chmod(0o1700)
    assert attrs(special)['rwx']=='rwx-----T'
    # Owner can stat a mode-000 file if the containing directory is searchable.
    f.chmod(0)
    assert attrs(f)['mode']=='0000'
    f.chmod(0o640)
    blocked=root/'blocked';blocked.mkdir();child=blocked/'item';child.touch()
    if os.geteuid()!=0:
        blocked.chmod(0o600)
        try:run('file_attr',child,status=1)
        finally:blocked.chmod(0o700)
    modes=root/'modes';modes.mkdir()
    out=run('mode_demo',modes)
    assert 'mask022.txt mode=0644' in out and 'after_fchmod mode=0640' in out
    assert 'mask077.txt mode=0600' in out and 'groupdir mode=0750' in out
    assert stat.S_IMODE((modes/'mask022.txt').stat().st_mode)==0o640
    run('mode_demo',modes,status=1)
    run('mode_demo',root/'absent',status=1)
    sparse=root/'sparse';run('sparse_demo',sparse)
    assert sparse.stat().st_size==1048577
    with sparse.open('rb') as stream:
        assert stream.read(16)==b'\0'*16
        stream.seek(1048576);assert stream.read()==b'X'
    run('sparse_demo',sparse,status=1)
    stamp=root/'times';out=run('time_attrs',stamp)
    assert 'after_futimens' in out and 'after_fchmod mode=0640' in out
    assert out.count('atime=1704067200 mtime=1704153600')==2
    st=stamp.stat()
    assert int(st.st_atime)==1704067200 and int(st.st_mtime)==1704153600
    assert stat.S_IMODE(st.st_mode)==0o640
    run('time_attrs',stamp,status=1)
    out=run('fd_unlink_demo')
    before=dict(re.findall(r'(\w+)=(\S+)',next(x for x in out.splitlines() if x.startswith('before '))))
    after=dict(re.findall(r'(\w+)=(\S+)',next(x for x in out.splitlines() if x.startswith('after '))))
    assert before['inode']==after['inode'] and before['device']==after['device']
    assert before['links']=='1' and after['links']=='0'
    assert before['size']==after['size']=='5'
    assert 'path_lookup=ENOENT' in out and 'read=hello' in out
    name=next(x[5:] for x in out.splitlines() if x.startswith('path='))
    assert not Path(name).exists()

for name in ('file_attr','mode_demo','sparse_demo','time_attrs'):
    run(name,status=2)
run('file_attr','--unknown','x',status=2)
run('fd_unlink_demo','extra',status=2)
print(f'PASS: {runs} process runs; attributes, links, permissions, sparse data, timestamps and unlink lifetime')
